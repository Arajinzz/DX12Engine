#pragma once

#include "Core/DX12Model.h"
#include "Core/DX12SwapChain.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace Core
{
  class DX12Context
  {
  public:
    DX12Context();
    ~DX12Context();

    void Present();
    void Execute();
    void WaitForGpu();
    void MoveToNextFrame();

    void Resize(unsigned width, unsigned height);

    void PrepareForRendering();
    void Draw(DX12Model* model);
    void PrepareForPresenting();

    ID3D12CommandQueue* GetCommandQueue() { return m_commandQueue.Get(); }
    ID3D12GraphicsCommandList* GetCommandList() { return m_commandList.Get(); }

  private:
    void InitFence();
    void SignalFence();
    void WaitFence();

  private:
    // the swapchain
    std::unique_ptr<DX12SwapChain> m_swapChain;
    
    // command queue
    ComPtr<ID3D12CommandQueue> m_commandQueue;
    // command list
    std::vector<ComPtr<ID3D12CommandAllocator>> m_commandAllocators;
    ComPtr<ID3D12GraphicsCommandList> m_commandList;
    // synchronization
    HANDLE m_fenceEvent;
    ComPtr<ID3D12Fence> m_fence;
    std::vector<uint64_t> m_fenceValues;

    // Synchronization objects.
    uint32_t m_frameIndex;

    // Rendering viewport and rect
    CD3DX12_VIEWPORT m_viewport;
    CD3DX12_RECT m_scissorRect;

  private:
    DX12Context(const DX12Context&) = delete;
    DX12Context& operator=(const DX12Context&) = delete;

  };
}

