#pragma once

#include "Core/ResourceManager.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace Core
{
  class DX12SwapChain
  {

  public:
    DX12SwapChain();
    ~DX12SwapChain();

    void Init(ID3D12CommandQueue* queue);
    void Present();
    void Resize(unsigned width, unsigned height);
    
    unsigned int GetCurrentBackBufferIndex();
    std::shared_ptr<RenderTargetDescriptor> GetRenderTarget(unsigned index) { return m_renderTargets[index]; }
    std::shared_ptr<RenderTargetDescriptor> GetCurrentRenderTarget() { return GetRenderTarget(GetCurrentBackBufferIndex()); }

  private:
    void CreateDepthResource();

    DX12SwapChain(const DX12SwapChain&) = delete;
    DX12SwapChain& operator= (const DX12SwapChain&) = delete;

  private:
    ComPtr<IDXGISwapChain3> m_swapChain;
    // rtv
    std::vector<std::shared_ptr<RenderTargetDescriptor>> m_renderTargets;
    // depth resource
    std::unique_ptr<ResourceDescriptor> m_depth;

  };
}

