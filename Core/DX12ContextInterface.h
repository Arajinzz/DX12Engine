#pragma once

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace Core
{
  class DX12ContextInterface
  {
  public:
    DX12ContextInterface();
    ~DX12ContextInterface();

  protected:
    void InitFence();
    void SignalFence(ID3D12CommandQueue* queue, int frameIndex);
    void WaitFence(int frameIndex);
    void ResetFence();
    void SetFenceValue(int frameIndex, uint64_t value);
    uint64_t GetFenceValue(int frameIndex);

  private:
    // synchronization
    HANDLE m_fenceEvent;
    ComPtr<ID3D12Fence> m_fence;
    std::vector<uint64_t> m_fenceValues;

  };
}
