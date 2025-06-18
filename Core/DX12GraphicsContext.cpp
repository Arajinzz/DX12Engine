#include "stdafx.h"
#include "DX12GraphicsContext.h"

#include "Core/WindowsApplication.h"
#include "Core/Application.h"
#include "Core/DX12FrameResource.h"
#include "Core/DX12Interface.h"
#include "Core/DX12Texture.h"

namespace Core
{
  DX12GraphicsContext::DX12GraphicsContext()
    : DX12ContextInterface()
    , m_frameIndex(0)
    , m_commandQueue(nullptr)
    , m_commandList(nullptr)
    , m_commandAllocators()
  {
    RECT rect;
    GetClientRect(WindowsApplication::GetHwnd(), &rect);
    auto width = rect.right - rect.left;
    auto height = rect.bottom - rect.top;
    m_viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
    m_scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(width), static_cast<LONG>(height));

    // create command queue
    m_commandQueue = DX12Interface::Get().CreateCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);

    // create command list and allocators for it
    for (unsigned n = 0; n < Application::FrameCount; ++n)
      m_commandAllocators.push_back(DX12Interface::Get().CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT));
    m_commandList = DX12Interface::Get().CreateCommandList(m_commandAllocators, D3D12_COMMAND_LIST_TYPE_DIRECT);

    FrameResource().GetSwapChain()->Init(m_commandQueue.Get());

    m_frameIndex = FrameResource().GetSwapChain()->GetCurrentBackBufferIndex();
  }

  DX12GraphicsContext::~DX12GraphicsContext()
  {
    m_commandList.Reset();
    m_commandQueue.Reset();
    m_commandAllocators.clear(); // could be a bug
  }

  void DX12GraphicsContext::Present()
  {
    FrameResource().GetSwapChain()->Present();
  }

  void DX12GraphicsContext::Execute()
  {
    // close command List
    m_commandList->Close();

    // execute commands to finish setup
    ID3D12CommandList* commandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(1, commandLists);
  }

  void DX12GraphicsContext::MoveToNextFrame()
  {
    // Signal and increment the fence value.
    SignalFence(m_commandQueue.Get(), m_frameIndex);

    // current frame Fence
    const UINT64 currentFenceValue = GetFenceValue(m_frameIndex);

    // next frame
    m_frameIndex = FrameResource().GetSwapChain()->GetCurrentBackBufferIndex();

    // If the next frame is not ready to be rendered yet, wait until
    WaitFence(m_frameIndex);

    // How I understand it is, the current frame will always a fenceValue bigger than next frame
    // Why is that? because we begin with fence values 0 for both frames, but the current frame
    // will be increment to one.
    // so we will have an initial state of
    // currentFrame fenceValue = 1
    // nextFrame fenceValue = 0
    // so if nextFrame if available it recieves current frame fence value
    // which will be
    // currentFrame fenceValue = 1
    // nextFrame fenceValue = 2
    SetFenceValue(m_frameIndex, currentFenceValue + 1);
  }

  void DX12GraphicsContext::Resize(unsigned width, unsigned height)
  {
    FrameResource().GetSwapChain()->Resize(width, height);

    m_viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
    m_scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(width), static_cast<LONG>(height));

    m_frameIndex = FrameResource().GetSwapChain()->GetCurrentBackBufferIndex();
    ResetFence();
    InitFence();
    m_frameIndex = FrameResource().GetSwapChain()->GetCurrentBackBufferIndex();
  }

  void DX12GraphicsContext::Reset()
  {
    // Command list allocators can only be reset when the associated 
    // command lists have finished execution on the GPU; apps should use 
    // fences to determine GPU execution progress.
    // However, when ExecuteCommandList() is called on a particular command 
    // list, that command list can then be reset at any time and must be before 
    // re-recording.
    ThrowIfFailed(m_commandAllocators[m_frameIndex]->Reset());
    ThrowIfFailed(m_commandList->Reset(m_commandAllocators[m_frameIndex].Get(), nullptr));
  }

  void DX12GraphicsContext::PrepareForRendering()
  {
    // Indicate that the back buffer will be used as a render target.
    auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(
      FrameResource().GetSwapChain()->GetRenderHeap()->GetResource(m_frameIndex), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    m_commandList->ResourceBarrier(1, &barrier1);

    // these must be done in the same commandlist as drawing
    // because they set a state for rendering
    // and states they reset between command lists
    // since we are using bundles for drawing this should be fine
    m_commandList->RSSetViewports(1, &m_viewport);
    m_commandList->RSSetScissorRects(1, &m_scissorRect);
    auto rtvHandle = FrameResource().GetSwapChain()->GetRenderHeap()->GetOffsetHandle(m_frameIndex);
    auto dsvHandle = FrameResource().GetSwapChain()->GetDepthHeap()->GetOffsetHandle(0);
    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

    // Record commands.
    const float clearColor[] = { 0.25f, 0.55f, 0.45f, 1.0f };
    m_commandList->ClearDepthStencilView(
      dsvHandle,
      D3D12_CLEAR_FLAG_DEPTH,
      1.0f,    // Clear depth to maximum (far plane)
      0,       // Clear stencil to 0
      0, nullptr
    );
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
  }

  void DX12GraphicsContext::Draw(DX12Mesh* mesh)
  {
    // RECHECK THIS!!!!!!!!!!!!!!!
    // set heaps, this has to be the same as bundles
    ID3D12DescriptorHeap* ppHeaps[] = { mesh->GetHeapDesc()->Get()};
    m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
    mesh->DrawMesh(m_frameIndex, m_commandList.Get()); // executes bundles
  }

  void DX12GraphicsContext::TransitionTextures(DX12Mesh* mesh)
  {
    ID3D12DescriptorHeap* ppHeaps[] = { mesh->GetHeapDesc()->Get() };
    m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
    // HACK, transition textures to shader resource on draw
    for (unsigned i = 0; i < mesh->GetTexturesCount(); ++i)
    {
      auto texture = mesh->GetTexture(i);
      auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(
        texture->GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
      m_commandList->ResourceBarrier(1, &barrier1);
    }
  }

  void DX12GraphicsContext::PrepareForPresenting()
  {
    // Indicate that the back buffer will now be used to present.
    auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(
      FrameResource().GetSwapChain()->GetRenderHeap()->GetResource(m_frameIndex), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    m_commandList->ResourceBarrier(1, &barrier1);
  }

  void DX12GraphicsContext::WaitForGpu()
  {
    // Wait for the command list to execute; we are reusing the same command
    // list in our main loop but for now, we just want to wait for setup to
    // complete before continuing.
    // Schedule a Signal command in the queue.
    SignalFence(m_commandQueue.Get(), m_frameIndex);
    WaitFence(m_frameIndex);
  }

}