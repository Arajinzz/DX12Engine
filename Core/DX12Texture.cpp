#include "stdafx.h"
#include "DX12Texture.h"

#include "Core/DX12Device.h"
#include "Core/DXApplicationHelper.h"
#include "Core/DX12FrameResource.h"

#include <DirectXTex/DirectXTex.h>

#include <filesystem>


namespace Core
{
  DX12Texture::DX12Texture(const char* path)
    : m_textureData()
    , m_path(path)
  {
    std::filesystem::path filePath(m_path);
    if (!std::filesystem::exists(filePath))
    {
      throw std::exception("File not found.");
    }

    TexMetadata metadata;
    ScratchImage scratchImage;

    // load texture
    unsigned char* img = stbi_load(path, &m_width, &m_height, &m_channels, 4);
    m_textureData.reserve(m_width * m_height * m_channels);
    memcpy(m_textureData.data(), img, m_width * m_height * m_channels * sizeof(uint8_t));
    stbi_image_free(img);

    D3D12_RESOURCE_DESC textureDesc = {};
    textureDesc.MipLevels = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.Width = m_width;
    textureDesc.Height = m_height;
    textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    textureDesc.DepthOrArraySize = 1;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    
    auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

    ThrowIfFailed(Device()->CreateCommittedResource(
      &heapProperties,
      D3D12_HEAP_FLAG_NONE,
      &textureDesc,
      D3D12_RESOURCE_STATE_COPY_DEST,
      nullptr,
      IID_PPV_ARGS(&m_texture)));

    const UINT subresourceCount = textureDesc.DepthOrArraySize * textureDesc.MipLevels;
    const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_texture.Get(), 0, subresourceCount);
    auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
    heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    ThrowIfFailed(Device()->CreateCommittedResource(
      &heapProperties,
      D3D12_HEAP_FLAG_NONE,
      &bufferDesc,
      D3D12_RESOURCE_STATE_GENERIC_READ,
      nullptr,
      IID_PPV_ARGS(&m_texUploadHeap)));
  }

  DX12Texture::~DX12Texture()
  {
  }

  void DX12Texture::CopyToGPU(ID3D12GraphicsCommandList* commandList)
  {
    // Copy data to the intermediate upload heap and then schedule 
    // a copy from the upload heap to the diffuse texture.
    D3D12_SUBRESOURCE_DATA textureData = {};
    textureData.pData = m_textureData.data();
    textureData.RowPitch = m_width * m_channels; // width * pixelSize
    textureData.SlicePitch = textureData.RowPitch * m_height; // rowpitch * height

    const UINT subresourceCount = 1 * 1;
    const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_texture.Get(), 0, subresourceCount);
    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    UpdateSubresources(commandList, m_texture.Get(), m_texUploadHeap.Get(), 0, 0, subresourceCount, &textureData);
    commandList->ResourceBarrier(1, &barrier);
  }
}
