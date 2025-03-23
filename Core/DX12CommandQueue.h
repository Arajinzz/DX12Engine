#pragma once

#include "Core/DX12CommandList.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace Core
{
  class DX12CommandQueue
  {
  public:
    DX12CommandQueue(bool isDirect = true);
    ~DX12CommandQueue();
    
    ID3D12CommandQueue* Get() { return m_commandQueue.Get(); }
    DX12CommandList* GetCommandList() { return m_commandList.get(); }
    
    void InitFence();

    void ExecuteCommandList();
    void ResetFence();

    void SignalFence(unsigned int frameIndex);
    void WaitFence(unsigned int frameIndex);
    void WaitForGpu(unsigned int frameIndex);
    unsigned int GetFenceValue(unsigned int frameIndex);
    void SetFenceValue(unsigned int frameIndex, unsigned int value);

  private:
    DX12CommandQueue(const DX12CommandQueue&) = delete;
    DX12CommandQueue& operator=(const DX12CommandQueue&) = delete;

  private:
    ComPtr<ID3D12CommandQueue> m_commandQueue;
    std::unique_ptr<DX12CommandList> m_commandList;

    // synchronization
    HANDLE m_fenceEvent;
    ComPtr<ID3D12Fence> m_fence;
    std::vector<uint64_t> m_fenceValues;
  };
}

