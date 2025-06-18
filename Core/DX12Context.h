#pragma once

#include "Core/DX12Heap.h"
#include "Core/DX12Mesh.h"
#include "Core/DX12SwapChain.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace Core
{
  enum
  {
    GenerateMipsCB,
    SrcMip,
    OutMip,
    NumRootParameters
  };

  struct alignas(16) SGenerateMipsCB
  {
    uint32_t SrcMipLevel;           // Texture level of source mip
    uint32_t NumMipLevels;          // Number of OutMips to write: [1-4]
    uint32_t SrcDimension;          // Width and height of the source texture are even or odd.
    uint32_t IsSRGB;                // Must apply gamma correction to sRGB textures.
    DirectX::XMFLOAT2 TexelSize;    // 1.0 / OutMip1.Dimensions
  };

  class DX12Context
  {
  public:
    DX12Context();
    ~DX12Context();

    void Present();
    void Execute();
    void WaitForGpu();
    void MoveToNextFrame();

    void CreateMips(DX12Mesh* mesh);

    void Resize(unsigned width, unsigned height);

    void PrepareForRendering();
    void Draw(DX12Mesh* mesh);
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

    // used for mipmaps
    std::unique_ptr<DX12Heap> m_mipsHeap;
    // pso used for mipmaps
    ComPtr<ID3D12PipelineState> m_pipelineState;
    // root sig used for mipmaps
    ComPtr<ID3D12RootSignature> m_rootSignature;

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

