#include "stdafx.h"
#include "DX12SwapChain.h"

#include "Core/WindowsApplication.h"
#include "Core/DXApplicationHelper.h"
#include "Core/DX12Device.h"
#include "Core/Application.h"

namespace Core
{
  DX12SwapChain::DX12SwapChain()
  {
  }

  DX12SwapChain::~DX12SwapChain()
  {
    m_swapChain.Reset();
  }

  void DX12SwapChain::Init(DX12CommandQueue* queue)
  {
    RECT rect;
    GetClientRect(WindowsApplication::GetHwnd(), &rect);
    // Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = Application::FrameCount;
    swapChainDesc.Width = rect.right - rect.left;
    swapChainDesc.Height = rect.bottom - rect.top;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> swapChain;
    ThrowIfFailed(Factory()->CreateSwapChainForHwnd(
      queue->Get(), // Swap chain needs the queue so that it can force a flush on it.
      WindowsApplication::GetHwnd(),
      &swapChainDesc,
      nullptr,
      nullptr,
      &swapChain
    ));
    ThrowIfFailed(swapChain.As(&m_swapChain));

    // Create RTV heap
    m_rtvHeap = std::make_unique<DX12Heap>(Application::FrameCount, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    // Create frame resources.
    m_rtvHeap->CreateResources();

    // Create DSV heap
    m_dsvHeap = std::make_unique<DX12Heap>(1, D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    // create resource
    m_dsvHeap->CreateResources();
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

  void DX12SwapChain::Resize(unsigned width, unsigned height)
  {
    // release render target
    m_rtvHeap->Reset();
    m_dsvHeap->Reset();

    auto hr = m_swapChain->ResizeBuffers(
      Application::FrameCount, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);

    m_rtvHeap->CreateResources();
    m_dsvHeap->CreateResources();

    if (FAILED(hr))
    {
      
    }
  }
}