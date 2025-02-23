#pragma once

using namespace DirectX;
using Microsoft::WRL::ComPtr;


#define CreateCmdQueue() DX12CommandQueue::Instance()
#define CommandQueue() DX12CommandQueue::Instance().GetCommandQueuePointer()

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

    ~DX12CommandQueue();

  private:
    DX12CommandQueue();
    DX12CommandQueue(const DX12CommandQueue&) = delete;
    DX12CommandQueue& operator=(const DX12CommandQueue&) = delete;

  private:
    ComPtr<ID3D12CommandQueue> m_commandQueue;
  };
}

