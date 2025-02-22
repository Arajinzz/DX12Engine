#pragma once

#include "DirectXApplication.h"
#include "DX12SwapChain.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace Core
{
  class Application : public DirectXApplication
  {
  public:
    Application(UINT width, UINT height, std::wstring name);

    virtual void OnInit() override;
    virtual void OnUpdate() override;
    virtual void OnRender() override;
    virtual void OnDestroy() override;

  private:
    static const uint32_t FrameCount = 2;

    // Pipeline objects
    std::unique_ptr<DX12SwapChain> m_swapChain;
    ComPtr<ID3D12CommandQueue> m_commandQueue;
    // render target heap
    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    unsigned int m_rtvDescriptorSize;
    ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
    ComPtr<ID3D12CommandAllocator> m_commandAllocators[FrameCount];
    ComPtr<ID3D12GraphicsCommandList> m_commandList;
    ComPtr<ID3D12RootSignature> m_rootSignature;
    ComPtr<ID3D12PipelineState> m_pipelineState;

    // Synchronization objects.
    uint32_t m_frameIndex;
    HANDLE m_fenceEvent;
    ComPtr<ID3D12Fence> m_fence;
    uint64_t m_fenceValues[FrameCount];

  };
}

