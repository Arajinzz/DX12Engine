#include "stdafx.h"
#include "DX12Context.h"

#include "Core/WindowsApplication.h"
#include "Core/Application.h"
#include "Core/DX12FrameResource.h"

namespace Core
{
  DX12Context::DX12Context()
    : m_frameIndex(0)
    , m_commandQueue(nullptr)
    , m_commandList(nullptr)
  {
    RECT rect;
    GetClientRect(WindowsApplication::GetHwnd(), &rect);
    auto width = rect.right - rect.left;
    auto height = rect.bottom - rect.top;
    m_viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
    m_scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(width), static_cast<LONG>(height));

    // create command queue
    m_commandQueue = std::make_unique<DX12CommandQueue>();
    m_commandList = m_commandQueue->GetCommandList();

    // Create synchronization objects.
    m_commandQueue->InitFence();
  }

  DX12Context::~DX12Context()
  {
  }

  void DX12Context::Init()
  {
    m_frameIndex = SwapChain().GetCurrentBackBufferIndex();
  }

  void DX12Context::Execute()
  {
    // close command List
    m_commandList->Close();
    // execute commands to finish setup
    m_commandQueue->ExecuteCommandList();
  }

  void DX12Context::WaitForGpu()
  {
    m_commandQueue->WaitForGpu(m_frameIndex);
  }

  void DX12Context::MoveToNextFrame()
  {
    // Signal and increment the fence value.
    m_commandQueue->SignalFence(m_frameIndex);

    // current frame Fence
    const UINT64 currentFenceValue = m_commandQueue->GetFenceValue(m_frameIndex);

    // next frame
    m_frameIndex = SwapChain().GetCurrentBackBufferIndex();

    // If the next frame is not ready to be rendered yet, wait until
    m_commandQueue->WaitFence(m_frameIndex);

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
    m_commandQueue->SetFenceValue(m_frameIndex, currentFenceValue + 1);
  }

  void DX12Context::Resize(unsigned width, unsigned height)
  {
    m_viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
    m_scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(width), static_cast<LONG>(height));

    m_frameIndex = SwapChain().GetCurrentBackBufferIndex();
    
    m_commandQueue->ResetFence();
  }

  void DX12Context::PrepareForRendering()
  {
    // Command list allocators can only be reset when the associated 
    // command lists have finished execution on the GPU; apps should use 
    // fences to determine GPU execution progress.
    // However, when ExecuteCommandList() is called on a particular command 
    // list, that command list can then be reset at any time and must be before 
    // re-recording.
    m_commandList->Reset(m_frameIndex, nullptr);

    // Indicate that the back buffer will be used as a render target.
    m_commandList->Transition(SwapChain().GetRenderHeap()->GetResource(m_frameIndex), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

    // set heaps, this has to be the same as bundles
    m_commandQueue->GetCommandList()->SetDescriptorHeap(FrameResource().GetHeap());

    // these must be done in the same commandlist as drawing
    // because they set a state for rendering
    // and states they reset between command lists
    // since we are using bundles for drawing this should be fine
    m_commandList->Get()->RSSetViewports(1, &m_viewport);
    m_commandList->Get()->RSSetScissorRects(1, &m_scissorRect);
    auto rtvHandle = SwapChain().GetRenderHeap()->GetOffsetHandle(m_frameIndex);
    auto dsvHandle = SwapChain().GetDepthHeap()->GetOffsetHandle(0);
    m_commandList->Get()->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

    // Record commands.
    const float clearColor[] = { 0.25f, 0.55f, 0.45f, 1.0f };
    m_commandList->ClearDepthStencilView(dsvHandle);
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor);
  }

  void DX12Context::Draw(DX12Cube* cube)
  {
    cube->Draw(m_frameIndex);
    m_commandList->Get()->ExecuteBundle(cube->GetBundle());
  }

  void DX12Context::PrepareForPresenting()
  {
    // Indicate that the back buffer will now be used to present.
    m_commandList->Transition(SwapChain().GetRenderHeap()->GetResource(m_frameIndex), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
  }
}