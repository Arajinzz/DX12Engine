#pragma once

using namespace DirectX;
using Microsoft::WRL::ComPtr;

#define CreateCmdQueue() DX12CommandQueue::Instance()
#define CommandQueue() DX12CommandQueue::Instance()

// not good, but will be simple for me to understand
namespace Core
{
  class DX12CommandQueue
  {
  public:
    static DX12CommandQueue& Instance()
    {
      static DX12CommandQueue instance;
      return instance;
    }

    ID3D12CommandQueue* GetCommandQueuePointer()
    {
      return m_commandQueue.Get();
    }

    void InitFence(unsigned int valuesCount);

    void AddCommandList(ID3D12CommandList* commandList);
    void RemoveCommandList(ID3D12CommandList* commandList);

    void ExecuteCommandLists();

    void SignalFence(unsigned int frameIndex);
    void WaitFence(unsigned int frameIndex);
    void WaitForGpu(unsigned int frameIndex);
    unsigned int GetFenceValue(unsigned int frameIndex);
    void SetFenceValue(unsigned int frameIndex, unsigned int value);

    ~DX12CommandQueue();

  private:
    DX12CommandQueue();
    DX12CommandQueue(const DX12CommandQueue&) = delete;
    DX12CommandQueue& operator=(const DX12CommandQueue&) = delete;

  private:
    ComPtr<ID3D12CommandQueue> m_commandQueue;
    std::vector<ID3D12CommandList*> m_commandLists;

    // synchronization
    HANDLE m_fenceEvent;
    ComPtr<ID3D12Fence> m_fence;
    std::vector<uint64_t> m_fenceValues;
  };
}

