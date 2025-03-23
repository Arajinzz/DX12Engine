#pragma once

#include "Core/DX12CommandQueue.h"

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

  class DX12MipsGenerator
  {
  public:
    DX12MipsGenerator();
    ~DX12MipsGenerator();

  private:
    // command queue
    std::unique_ptr<DX12CommandQueue> m_commandQueue;
    // heap
    std::unique_ptr<DX12Heap> m_uavHeap;
    // pso
    ComPtr<ID3D12PipelineState> m_pipelineState;
    // root sig
    ComPtr<ID3D12RootSignature> m_rootSignature;

  private:
    DX12MipsGenerator(const DX12MipsGenerator&) = delete;
    DX12MipsGenerator& operator=(const DX12MipsGenerator&) = delete;
  };
}

