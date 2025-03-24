#pragma once

#include "Core/DX12CommandQueue.h"
#include "Core/DX12CommandList.h"
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

  struct alignas(16) GenerateMipsCB
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

    DX12CommandQueue* GetCommandQueue() { return m_commandQueue.get(); }
    DX12CommandList* GetCommandList() { return m_commandList; }

  private:
    // the swapchain
    std::unique_ptr<DX12SwapChain> m_swapChain;
    // command queue
    std::unique_ptr<DX12CommandQueue> m_commandQueue;
    // command list associated to command queue
    DX12CommandList* m_commandList; // owned by command queue
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

