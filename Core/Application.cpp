#include "stdafx.h"
#include "Application.h"

#include "Core/DX12Device.h"
#include "Core/DX12CommandQueue.h"

namespace Core
{
  Application::Application(UINT width, UINT height, std::wstring name)
    : DirectXApplication(width, height, name)
    , m_frameIndex(0)
    , m_swapChain(nullptr)
  {
  }

  void Application::OnInit()
  {
    // Load pipeline

    // Create Device
    CreateDevice(); // Singleton

    // Create Command Queue
    CreateCmdQueue(); // Singleton

    // Create SwapChain
    m_swapChain = std::make_unique<DX12SwapChain>(FrameCount, m_width, m_height);
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    // full screen transitions not supported.
    ThrowIfFailed(Factory()->MakeWindowAssociation(WindowsApplication::GetHwnd(), DXGI_MWA_NO_ALT_ENTER));

    // Create RTV heap
    m_rtvHeap = std::make_unique<DX12Heap>(FrameCount, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, m_swapChain.get());

    // Create frame resources.
    m_rtvHeap->CreateResources();

    // Create the command list.
    for (unsigned n = 0; n < FrameCount; ++n)
    {
      ThrowIfFailed(Device()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[n])));
      ThrowIfFailed(Device()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[n].Get(), nullptr, IID_PPV_ARGS(&m_commandList)));
    }

    // Close the command list and execute it to begin the initial GPU setup.
    ThrowIfFailed(m_commandList->Close());
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    CommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Create synchronization objects.
    {
      ThrowIfFailed(Device()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
      m_fenceValues[m_frameIndex]++;

      // Create an event handle to use for frame synchronization.
      m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
      if (m_fenceEvent == nullptr)
      {
        ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
      }

      // Wait for the command list to execute; we are reusing the same command 
      // list in our main loop but for now, we just want to wait for setup to 
      // complete before continuing.
      // Schedule a Signal command in the queue.
      ThrowIfFailed(CommandQueue()->Signal(m_fence.Get(), m_fenceValues[m_frameIndex]));

      // Wait until the fence has been processed.
      ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent));
      WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);

      // Increment the fence value for the current frame.
      m_fenceValues[m_frameIndex]++;
    }
  }

  void Application::OnUpdate()
  {
  }

  void Application::OnRender()
  {
    // Populate Command list
    // Command list allocators can only be reset when the associated 
    // command lists have finished execution on the GPU; apps should use 
    // fences to determine GPU execution progress.
    ThrowIfFailed(m_commandAllocators[m_frameIndex]->Reset());

    // However, when ExecuteCommandList() is called on a particular command 
    // list, that command list can then be reset at any time and must be before 
    // re-recording.
    ThrowIfFailed(m_commandList->Reset(m_commandAllocators[m_frameIndex].Get(), nullptr));

    auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(m_rtvHeap->GetResource(m_frameIndex), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    // Indicate that the back buffer will be used as a render target.
    m_commandList->ResourceBarrier(1, &barrier1);

    // Record commands.
    const float clearColor[] = { 0.25f, 0.25f, 0.45f, 1.0f };
    m_commandList->ClearRenderTargetView(m_rtvHeap->GetOffsetHandle(m_frameIndex), clearColor, 0, nullptr);

    auto barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(m_rtvHeap->GetResource(m_frameIndex), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    // Indicate that the back buffer will now be used to present.
    m_commandList->ResourceBarrier(1, &barrier2);

    ThrowIfFailed(m_commandList->Close());


    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    CommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Present the frame.
    m_swapChain->Present();

    // MOVE TO NEXT FRAME
    // Signal and increment the fence value.
    const UINT64 currentFenceValue = m_fenceValues[m_frameIndex];
    ThrowIfFailed(CommandQueue()->Signal(m_fence.Get(), currentFenceValue));

    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    // If the next frame is not ready to be rendered yet, wait until
    if (m_fence->GetCompletedValue() < m_fenceValues[m_frameIndex])
    {
      ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent));
      WaitForSingleObjectEx(m_fenceEvent, INFINITE, false);
    }

    m_fenceValues[m_frameIndex] = currentFenceValue + 1;
  }

  void Application::OnDestroy()
  {
    // Ensure that the GPU is no longer referencing resources that are about to be
    // cleaned up by the destructor.

    // Wait for the command list to execute; we are reusing the same command 
    // list in our main loop but for now, we just want to wait for setup to 
    // complete before continuing.
    // Schedule a Signal command in the queue.
    ThrowIfFailed(CommandQueue()->Signal(m_fence.Get(), m_fenceValues[m_frameIndex]));

    // Wait until the fence has been processed.
    ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent));
    WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);

    // Increment the fence value for the current frame.
    m_fenceValues[m_frameIndex]++;

    CloseHandle(m_fenceEvent);
  }
}