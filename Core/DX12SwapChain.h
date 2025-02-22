#pragma once

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace Core
{
  class DX12SwapChain
  {

  public:
    DX12SwapChain(unsigned int frameCount, unsigned int width, unsigned int height, ID3D12CommandQueue* cmdQueue);
    ~DX12SwapChain();

    unsigned int GetCurrentBackBufferIndex();
    void GetBuffer(unsigned int n, Microsoft::WRL::Details::ComPtrRef<Microsoft::WRL::ComPtr<ID3D12Resource>> renderTarget);
    void Present();

  private:
    DX12SwapChain(const DX12SwapChain&) = delete;
    DX12SwapChain& operator= (const DX12SwapChain&) = delete;

  private:
    ComPtr<IDXGISwapChain3> m_swapChain;

  };
}

