#include "stdafx.h"
#include "DX12Texture.h"

#include "Core/DX12Device.h"
#include "Core/DXApplicationHelper.h"

namespace
{
  std::vector<UINT8> GenerateTextureData(unsigned texW, unsigned texH, unsigned pixelSize)
  {
    const UINT rowPitch = texW * pixelSize;
    const UINT cellPitch = rowPitch >> 3; // The width of a cell in the checkboard texture.
    const UINT cellHeight = texW >> 3;    // The height of a cell in the checkerboard texture.
    const UINT textureSize = rowPitch * texH;

    std::vector<UINT8> data(textureSize);
    UINT8* pData = &data[0];

    for (UINT n = 0; n < textureSize; n += pixelSize)
    {
      UINT x = n % rowPitch;
      UINT y = n / rowPitch;
      UINT i = x / cellPitch;
      UINT j = y / cellHeight;

      if (i % 2 == j % 2)
      {
        pData[n] = 0x00;        // R
        pData[n + 1] = 0x00;    // G
        pData[n + 2] = 0x00;    // B
        pData[n + 3] = 0xff;    // A
      } else
      {
        pData[n] = 0xff;        // R
        pData[n + 1] = 0xff;    // G
        pData[n + 2] = 0xff;    // B
        pData[n + 3] = 0xff;    // A
      }
    }

    return data;
  }
}

namespace Core
{
  DX12Texture::DX12Texture(DX12Heap* heap)
  {
    D3D12_RESOURCE_DESC textureDesc = {};
    textureDesc.MipLevels = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.Width = 256;
    textureDesc.Height = 256;
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

    heap->AddResource(m_texture, TEXTURE);
  }

  DX12Texture::~DX12Texture()
  {
  }

  void DX12Texture::Init(ID3D12GraphicsCommandList* commandList)
  {
    // Copy data to the intermediate upload heap and then schedule 
    // a copy from the upload heap to the diffuse texture.
    std::vector<UINT8> texture = GenerateTextureData(256, 256, 4);
    D3D12_SUBRESOURCE_DATA textureData = {};
    textureData.pData = texture.data();
    textureData.RowPitch = 256 * 4; // width * pixelSize
    textureData.SlicePitch = textureData.RowPitch * 256; // rowpitch * height

    const UINT subresourceCount = 1 * 1;
    const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_texture.Get(), 0, subresourceCount);
    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    UpdateSubresources(commandList, m_texture.Get(), m_texUploadHeap.Get(), 0, 0, subresourceCount, &textureData);
    commandList->ResourceBarrier(1, &barrier);
  }
}
