#pragma once

#include "Core/ResourceManager.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace Core
{
  class DX12Texture
  {
  public:
    DX12Texture(std::vector<std::string> paths, unsigned mips = 4);
    ~DX12Texture();

    unsigned GetMipsLevels() { return m_mipsLevels; }
    TextureDescriptor* GetResource() { return m_texture.get(); }
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
    std::shared_ptr<TextureDescriptor> m_texture;
    
    std::vector<unsigned char*> m_imgPtrs;
    std::vector<MetaData> m_metaData;
    unsigned m_mipsLevels;

    // indicate that this texture was already uploaded to the GPU
    bool m_uploaded;
    // indicate that mips were already generated for this texture
    bool m_mipsGenerated;

  private:
    DX12Texture(const DX12Texture&) = delete;
    DX12Texture& operator=(const DX12Texture&) = delete;
  };
}

