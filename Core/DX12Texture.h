#pragma once

#include "Core/DX12Heap.h"
#include "Core/ResourceManager.h"

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

  struct SGenerateMipsCB
  {
    uint32_t SrcMipLevel;           // Texture level of source mip
    uint32_t NumMipLevels;          // Number of OutMips to write: [1-4]
    uint32_t SrcDimension;          // Width and height of the source texture are even or odd.
    uint32_t IsSRGB;                // Must apply gamma correction to sRGB textures.
    DirectX::XMFLOAT2 TexelSize;    // 1.0 / OutMip1.Dimensions
  };

  class DX12Texture
  {
  public:
    DX12Texture(std::vector<std::string> paths, unsigned mips = 4);
    ~DX12Texture();

    unsigned GetMipsLevels() { return m_mipsLevels; }
    ID3D12Resource* GetResource() { return m_texture.resource.Get(); }
    void CopyToGPU(ID3D12GraphicsCommandList* commandList);
    void GenerateMips(ID3D12GraphicsCommandList* commandList);

  private:
    struct MetaData
    {
      int width;
      int height;
      int channels;
    };

    // descriptor to the heap
    TextureDescriptor m_texture;

    // pso used for mipmaps
    ComPtr<ID3D12PipelineState> m_pipelineState;
    // root sig used for mipmaps
    ComPtr<ID3D12RootSignature> m_rootSignature;
    
    std::vector<unsigned char*> m_imgPtrs;
    std::vector<MetaData> m_metaData;
    unsigned m_mipsLevels;

  private:
    DX12Texture(const DX12Texture&) = delete;
    DX12Texture& operator=(const DX12Texture&) = delete;
  };
}

