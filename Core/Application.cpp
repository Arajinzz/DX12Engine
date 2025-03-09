#include "stdafx.h"
#include "Application.h"

#include "Core/DX12Device.h"
#include "Core/DX12CommandQueue.h"
#include "Core/DX12FrameResource.h"

namespace Core
{
  Application::Application(UINT width, UINT height, std::wstring name)
    : DirectXApplication(width, height, name)
    , m_frameIndex(0)
    , m_commandQueue(nullptr)
    , m_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height))
    , m_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height))
  {
  }

  void Application::OnInit()
  {
    LoadPipeline();
    
    // Create synchronization objects.
    m_commandQueue->InitFence(FrameCount);

    CreateFrameResource();

    // Create Cube, will also create pso and root signature and constant buffer for transformation
    cubes.push_back(new DX12Cube(GetWidth(), GetHeight(), 0.0));
    cubes.push_back(new DX12Cube(GetWidth(), GetHeight(), 0.5));
    cubes.push_back(new DX12Cube(GetWidth(), GetHeight(), -0.5));

    FrameResource().Init(m_commandQueue->GetCommandList()->Get());

    m_commandQueue->GetCommandList()->Close();
    // execute commands to finish setup
    m_commandQueue->ExecuteCommandList();
    // Wait for GPU to finish Execution
    m_commandQueue->WaitForGpu(m_frameIndex);
  }

  void Application::OnUpdate()
  {
    FrameResource().Update();

    for (auto cube : cubes)
      cube->Update();
  }

  void Application::OnRender()
  {
    // Populate Command list
    // Command list allocators can only be reset when the associated 
    // command lists have finished execution on the GPU; apps should use 
    // fences to determine GPU execution progress.
    // However, when ExecuteCommandList() is called on a particular command 
    // list, that command list can then be reset at any time and must be before 
    // re-recording.
    m_commandQueue->GetCommandList()->Reset(m_frameIndex, nullptr);

    // Indicate that the back buffer will be used as a render target.
    m_commandQueue->GetCommandList()->Transition(m_rtvHeap->GetResource(m_frameIndex), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

    // set heaps, this has to be the same as bundles
    m_commandQueue->GetCommandList()->SetDescriptorHeap(FrameResource().GetHeap());

    // these must be done in the same commandlist as drawing
    // because they set a state for rendering
    // and states they reset between command lists
    // since we are using bundles for drawing this should be fine
    m_commandQueue->GetCommandList()->Get()->RSSetViewports(1, &m_viewport);
    m_commandQueue->GetCommandList()->Get()->RSSetScissorRects(1, &m_scissorRect);
    auto rtvHandle = m_rtvHeap->GetOffsetHandle(m_frameIndex);
    auto dsvHandle = m_dsvHeap->GetOffsetHandle(0);
    m_commandQueue->GetCommandList()->Get()->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

    // Record commands.
    const float clearColor[] = { 0.25f, 0.55f, 0.45f, 1.0f };
    m_commandQueue->GetCommandList()->ClearDepthStencilView(m_dsvHeap->GetOffsetHandle(0));
    m_commandQueue->GetCommandList()->ClearRenderTargetView(m_rtvHeap->GetOffsetHandle(m_frameIndex), clearColor);

    // draw cube
    for (auto cube : cubes)
    {
      cube->Draw(m_rtvHeap.get(), m_dsvHeap.get(), m_frameIndex);
      m_commandQueue->GetCommandList()->Get()->ExecuteBundle(cube->GetBundle());
    }

    // Indicate that the back buffer will now be used to present.
    m_commandQueue->GetCommandList()->Transition(m_rtvHeap->GetResource(m_frameIndex), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

    m_commandQueue->GetCommandList()->Close();

    m_commandQueue->ExecuteCommandList();

    // Present the frame.
    SwapChain().Present();

    MoveToNextFrame(); // try to render next frame
  }

  void Application::OnDestroy()
  {
    // Ensure that the GPU is no longer referencing resources that are about to be
    // cleaned up by the destructor.
    // Wait for the command list to execute; we are reusing the same command 
    // list in our main loop but for now, we just want to wait for setup to 
    // complete before continuing.
    // Schedule a Signal command in the queue.
    m_commandQueue->WaitForGpu(m_frameIndex);
  }

  void Application::LoadPipeline()
  {
    // Create Device
    CreateDevice(); // Singleton

    // Create Command Queue
    m_commandQueue = std::make_unique<DX12CommandQueue>();

    // Create SwapChain
    CreateSwapChain(m_commandQueue.get());
    m_frameIndex = SwapChain().GetCurrentBackBufferIndex();

    // full screen transitions not supported.
    ThrowIfFailed(Factory()->MakeWindowAssociation(WindowsApplication::GetHwnd(), DXGI_MWA_NO_ALT_ENTER));

    // Create RTV heap
    m_rtvHeap = std::make_unique<DX12Heap>(FrameCount, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    // Create frame resources.
    m_rtvHeap->CreateResources();

    // Create DSV heap
    m_dsvHeap = std::make_unique<DX12Heap>(1, D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    // create resource
    m_dsvHeap->CreateResources();
  }

  void Application::MoveToNextFrame()
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
}