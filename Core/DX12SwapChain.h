#pragma once

#include "Core/DX12CommandQueue.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace Core
{
  class DX12SwapChain
  {

  public:
    DX12SwapChain();
    ~DX12SwapChain();

    DX12Heap* GetRenderHeap() { return m_rtvHeap.get(); }
    DX12Heap* GetDepthHeap() { return m_dsvHeap.get(); }
    void Init(DX12CommandQueue* queue);
    unsigned int GetCurrentBackBufferIndex();
    void GetBuffer(unsigned int n, Microsoft::WRL::Details::ComPtrRef<Microsoft::WRL::ComPtr<ID3D12Resource>> renderTarget);
    void Present();
    void Resize(unsigned width, unsigned height);

  private:
    DX12SwapChain(const DX12SwapChain&) = delete;
    DX12SwapChain& operator= (const DX12SwapChain&) = delete;

  private:
    ComPtr<IDXGISwapChain3> m_swapChain;
    // render target heap
    std::unique_ptr<DX12Heap> m_rtvHeap;
    // depth heap
    std::unique_ptr<DX12Heap> m_dsvHeap;

  };
}

