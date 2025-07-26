#pragma once

#include "Scene/DX12Model.h"
#include "Scene/DX12Skybox.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace Graphics
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

    void BeginFrame();
    void Draw(DX12Model* model, ID3D12PipelineState* pso, ID3D12RootSignature* rootSig);
    void EndFrame();

    ID3D12CommandQueue* GetCommandQueue() { return m_commandQueue.Get(); }
    ID3D12GraphicsCommandList* GetCommandList() { return m_commandList.Get(); }
    RenderTargetDescriptor* GetCurrentRenderTarget() { return m_renderTargets[m_frameIndex].get(); }

  private:
    void InitFence();
    void SignalFence();
    void WaitFence();

  private:
    // the swapchain
    ComPtr<IDXGISwapChain3> m_swapChain;
    std::vector<std::shared_ptr<RenderTargetDescriptor>> m_renderTargets;
    std::unique_ptr<ResourceDescriptor> m_depth;

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

