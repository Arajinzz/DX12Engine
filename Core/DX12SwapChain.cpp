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
    m_rtvHeap = std::make_unique<DX12Heap>(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    // Add render targets
    for (unsigned i = 0; i < Application::FrameCount; ++i)
    {
      ComPtr<ID3D12Resource> renderTarget;
      GetBuffer(i, &renderTarget);
      m_rtvHeap->AddResource(renderTarget, RENDERTARGET);
    }

    // Create DSV heap
    m_dsvHeap = std::make_unique<DX12Heap>(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    // create depth resource
    CreateDepthResource();
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
    m_rtvHeap->ResetResources();
    m_dsvHeap->ResetResources();

    auto hr = m_swapChain->ResizeBuffers(
      Application::FrameCount, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);

    // Add render targets
    for (unsigned i = 0; i < Application::FrameCount; ++i)
    {
      ComPtr<ID3D12Resource> renderTarget;
      GetBuffer(i, &renderTarget);
      m_rtvHeap->AddResource(renderTarget, RENDERTARGET);
    }

    CreateDepthResource();

    if (FAILED(hr))
    {
      
    }
  }

  void DX12SwapChain::CreateDepthResource()
  {
    ComPtr<ID3D12Resource> renderTarget;
    GetBuffer(0, &renderTarget); // any render target
    // Create resouce
    D3D12_RESOURCE_DESC depthStencilDesc = {};
    depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthStencilDesc.Alignment = 0;
    depthStencilDesc.Width = renderTarget->GetDesc().Width;  // Swap chain width
    depthStencilDesc.Height = renderTarget->GetDesc().Height; // Swap chain height
    depthStencilDesc.DepthOrArraySize = 1;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthStencilDesc.SampleDesc.Count = 1; // No multi-sampling
    depthStencilDesc.SampleDesc.Quality = 0;
    depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = DXGI_FORMAT_D32_FLOAT;
    clearValue.DepthStencil.Depth = 1.0f;
    clearValue.DepthStencil.Stencil = 0;

    auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    ThrowIfFailed(Device()->CreateCommittedResource(
      &heapProp,
      D3D12_HEAP_FLAG_NONE,
      &depthStencilDesc,
      D3D12_RESOURCE_STATE_DEPTH_WRITE,
      &clearValue,
      IID_PPV_ARGS(&m_depthResource)));

    m_dsvHeap->AddResource(m_depthResource, DEPTH);
  }
}