#pragma once

#include "Core/DX12Heap.h"
#include "Core/DX12Mesh.h"
#include "Core/DX12SwapChain.h"
#include "Core/DX12ContextInterface.h"

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

  class DX12ComputeContext : public DX12ContextInterface
  {
  public:
    DX12ComputeContext();
    ~DX12ComputeContext();

    void Execute();
    void MoveToNextFrame();

    void CreateMips(DX12Mesh* mesh);
    void WaitForGpu();

    ID3D12CommandQueue* GetCommandQueue() { return m_commandQueue.Get(); }
    ID3D12CommandList* GetCommandList() { return m_commandList.Get(); }

  private:
    // command queue
    ComPtr<ID3D12CommandQueue> m_commandQueue;
    // command list
    std::vector<ComPtr<ID3D12CommandAllocator>> m_commandAllocators;
    ComPtr<ID3D12GraphicsCommandList> m_commandList;

    // used for mipmaps
    std::unique_ptr<DX12Heap> m_mipsHeap;
    // pso used for mipmaps
    ComPtr<ID3D12PipelineState> m_pipelineState;
    // root sig used for mipmaps
    ComPtr<ID3D12RootSignature> m_rootSignature;

    // Synchronization objects.
    uint32_t m_frameIndex;

  private:
    DX12ComputeContext(const DX12ComputeContext&) = delete;
    DX12ComputeContext& operator=(const DX12ComputeContext&) = delete;

  };
}
