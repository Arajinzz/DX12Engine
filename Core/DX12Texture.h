#pragma once

#include "Core/DX12CommandList.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace Core
{
  class DX12Texture
  {
  public:
    DX12Texture(const char* path);
    ~DX12Texture();

    ID3D12Resource* GetResource() { return m_texture.Get(); }

    void Init(ID3D12GraphicsCommandList* commandList);

  private:
    ComPtr<ID3D12Resource> m_texture;
    ComPtr<ID3D12Resource> m_texUploadHeap;
    std::vector<uint8_t> m_textureData;

    // metadata
    int m_width;
    int m_height;
    int m_channels;

  private:
    DX12Texture(const DX12Texture&) = delete;
    DX12Texture& operator=(const DX12Texture&) = delete;
  };
}

