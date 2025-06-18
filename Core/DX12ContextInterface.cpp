#include "stdafx.h"
#include "DX12ContextInterface.h"

#include "Core/DXApplicationHelper.h"
#include "Core/Application.h"
#include "Core/DX12Interface.h"

namespace Core
{
  DX12ContextInterface::DX12ContextInterface()
    : m_fence(nullptr)
    , m_fenceEvent()
    , m_fenceValues()
  {
    // Create synchronization objects.
    InitFence();
  }

  DX12ContextInterface::~DX12ContextInterface()
  {
    m_fence.Reset();
    m_fenceValues.clear();
  }

  void DX12ContextInterface::InitFence()
  {
    m_fence = DX12Interface::Get().CreateFence();
    for (unsigned n = 0; n < Application::FrameCount; ++n)
      m_fenceValues.push_back(0);
    m_fenceValues[0]++; // start from buffer number 0

    m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (m_fenceEvent == nullptr)
      ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
  }

  void DX12ContextInterface::SignalFence(ID3D12CommandQueue* queue, int frameIndex)
  {
    ThrowIfFailed(queue->Signal(m_fence.Get(), m_fenceValues[frameIndex]));
  }

  void DX12ContextInterface::WaitFence(int frameIndex)
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

  void DX12ContextInterface::ResetFence()
  {
    // reset fence
    m_fence.Reset();
    m_fenceValues.clear();
  }

  void DX12ContextInterface::SetFenceValue(int frameIndex, uint64_t value)
  {
    m_fenceValues[frameIndex] = value;
  }

  uint64_t DX12ContextInterface::GetFenceValue(int frameIndex)
  {
    return m_fenceValues[frameIndex];
  }
}
