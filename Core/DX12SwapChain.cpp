#include "stdafx.h"
#include "DX12SwapChain.h"

#include "Core/WindowsApplication.h"
#include "Core/DXApplicationHelper.h"
#include "Core/DX12Device.h"
#include "Core/DX12CommandQueue.h"

namespace Core
{
  DX12SwapChain::DX12SwapChain(unsigned int frameCount, unsigned int width, unsigned int height)
  {
    // Describe and create the swap chain.
    {
      DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
      swapChainDesc.BufferCount = frameCount;
      swapChainDesc.Width = width;
      swapChainDesc.Height = height;
      swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
      swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
      swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
      swapChainDesc.SampleDesc.Count = 1;

      ComPtr<IDXGISwapChain1> swapChain;
      ThrowIfFailed(Factory()->CreateSwapChainForHwnd(
        CommandQueue(), // Swap chain needs the queue so that it can force a flush on it.
        WindowsApplication::GetHwnd(),
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain
      ));
      ThrowIfFailed(swapChain.As(&m_swapChain));
    }
  }

  DX12SwapChain::~DX12SwapChain()
  {
    m_swapChain.Reset();
  }

  unsigned int DX12SwapChain::GetCurrentBackBufferIndex()
  {
    return m_swapChain->GetCurrentBackBufferIndex();
  }

  void DX12SwapChain::GetBuffer(unsigned int n, Microsoft::WRL::Details::ComPtrRef<Microsoft::WRL::ComPtr<ID3D12Resource>> renderTarget)
  {
    ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(renderTarget)));
  }

  void DX12SwapChain::Present()
  {
    // no vsync
    ThrowIfFailed(m_swapChain->Present(0, 0));
  }
}