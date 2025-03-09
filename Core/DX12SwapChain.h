#pragma once

#include "Core/DX12CommandQueue.h"

#define CreateSwapChain(Queue) DX12SwapChain::Instance().Init(Queue)
#define SwapChain() DX12SwapChain::Instance()

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace Core
{
  class DX12SwapChain
  {

  public:
    static DX12SwapChain& Instance()
    {
      static DX12SwapChain instance;
      return instance;
    }
    ~DX12SwapChain();

    void Init(DX12CommandQueue* queue);
    unsigned int GetCurrentBackBufferIndex();
    void GetBuffer(unsigned int n, Microsoft::WRL::Details::ComPtrRef<Microsoft::WRL::ComPtr<ID3D12Resource>> renderTarget);
    void Present();

  private:
    DX12SwapChain();
    DX12SwapChain(const DX12SwapChain&) = delete;
    DX12SwapChain& operator= (const DX12SwapChain&) = delete;

  private:
    ComPtr<IDXGISwapChain3> m_swapChain;

  };
}

