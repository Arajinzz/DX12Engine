#pragma once

#include "Core/DX12CommandList.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace Core
{
  class DX12Texture
  {
  public:
    DX12Texture(std::vector<std::string> paths);
    ~DX12Texture();

    ID3D12Resource* GetResource() { return m_texture.Get(); }
    void CopyToGPU(ID3D12GraphicsCommandList* commandList);

  private:
    struct MetaData
    {
      int width;
      int height;
      int channels;
    };

    ComPtr<ID3D12Resource> m_texture;
    ComPtr<ID3D12Resource> m_texUploadHeap;
    
    std::vector<unsigned char*> m_imgPtrs;
    std::vector<MetaData> m_metaData;

  private:
    DX12Texture(const DX12Texture&) = delete;
    DX12Texture& operator=(const DX12Texture&) = delete;
  };
}

