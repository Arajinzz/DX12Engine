#include "stdafx.h"
#include "DX12CommandQueue.h"

#include "Core/DXApplicationHelper.h"
#include "Core/DX12Interface.h"

#include "Core/Application.h"

namespace Core
{
  DX12CommandQueue::DX12CommandQueue()
    : m_commandList(nullptr)
    , m_commandQueue(nullptr)
    , m_fenceValues()
  {
    // Describe and create the command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    ThrowIfFailed(DX12Interface::Get().GetDevice()->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

    m_commandList = std::make_unique<DX12CommandList>();
  }

  // values count represent how many framebuffer are used in the swapchain
  void DX12CommandQueue::InitFence()
  {
    // Create Fence
    ThrowIfFailed(DX12Interface::Get().GetDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));

    for (unsigned n = 0; n < Application::FrameCount; ++n)
      m_fenceValues.push_back(0);

    m_fenceValues[0]++; // start from buffer number 0

    // Create an event handle to use for frame synchronization.
    m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (m_fenceEvent == nullptr)
    {
      ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
    }
  }

  void DX12CommandQueue::ExecuteCommandList()
  {
    ID3D12CommandList* commandLists[] = { m_commandList->Get() };
    m_commandQueue->ExecuteCommandLists(1, commandLists);
  }

  void DX12CommandQueue::ResetFence()
  {
    m_fence.Reset();
    m_fenceValues.clear();
    InitFence();
  }

  void DX12CommandQueue::SignalFence(unsigned int frameIndex)
  {
    ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_fenceValues[frameIndex]));
  }

  void DX12CommandQueue::WaitFence(unsigned int frameIndex)
  {
    // gpu didn't finish yet
    auto completedValue = m_fence->GetCompletedValue();
    if (completedValue < m_fenceValues[frameIndex])
    {
      // Wait until the fence has been processed.
      ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[frameIndex], m_fenceEvent));
      WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
    }
    // Increment the fence value for the current frame.
    m_fenceValues[frameIndex]++;
  }

  void DX12CommandQueue::WaitForGpu(unsigned int frameIndex)
  {
    // Wait for the command list to execute; we are reusing the same command 
    // list in our main loop but for now, we just want to wait for setup to 
    // complete before continuing.
    // Schedule a Signal command in the queue.
    SignalFence(frameIndex);

    WaitFence(frameIndex);
  }

  unsigned int DX12CommandQueue::GetFenceValue(unsigned int frameIndex)
  {
    return m_fenceValues[frameIndex];
  }

  void DX12CommandQueue::SetFenceValue(unsigned int frameIndex, unsigned int value)
  {
    m_fenceValues[frameIndex] = value;
  }

  DX12CommandQueue::~DX12CommandQueue()
  {
    // maybe wait for GPU here
    CloseHandle(m_fenceEvent);
    m_commandQueue.Reset();
    m_fence.Reset();
  }
}