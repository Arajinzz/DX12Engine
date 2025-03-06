#pragma once

#include "Core/DX12CommandList.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace Core
{
  class DX12Texture
  {
  public:
    DX12Texture(DX12Heap* heap);
    ~DX12Texture();

    void Init(ID3D12GraphicsCommandList* commandList);

  private:
    ComPtr<ID3D12Resource> m_texture;
    ComPtr<ID3D12Resource> m_texUploadHeap;

  private:
    DX12Texture(const DX12Texture&) = delete;
    DX12Texture& operator=(const DX12Texture&) = delete;
  };
}

