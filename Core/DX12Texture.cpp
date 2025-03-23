#include "stdafx.h"
#include "DX12Texture.h"

#include "Core/DX12Device.h"
#include "Core/DXApplicationHelper.h"
#include "Core/DX12FrameResource.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace Core
{
  DX12Texture::DX12Texture(std::vector<std::string> paths)
    : m_imgPtrs()
    , m_metaData()
  {
    for (const auto& path : paths)
    {
      MetaData metadata;
      m_imgPtrs.emplace_back(stbi_load(path.c_str(), &metadata.width, &metadata.height, &metadata.channels, 4));
      metadata.channels = 4; // force to 4
      m_metaData.push_back(metadata);
    }

    D3D12_RESOURCE_DESC textureDesc = {};
    textureDesc.MipLevels = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.Width = m_metaData[0].width;
    textureDesc.Height = m_metaData[0].height;
    textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    textureDesc.DepthOrArraySize = m_imgPtrs.size();
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
    std::vector<D3D12_SUBRESOURCE_DATA> textureData(m_imgPtrs.size());

    for (unsigned i = 0; i < m_imgPtrs.size(); ++i)
    {
      textureData[i].pData = m_imgPtrs[i];
      textureData[i].RowPitch = m_metaData[i].width * m_metaData[i].channels; // width * pixelSize
      textureData[i].SlicePitch = textureData[i].RowPitch * m_metaData[i].height; // rowpitch * height
    }

    const UINT subResourceCount = 1 * m_metaData.size();
    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    UpdateSubresources(commandList, m_texture.Get(), m_texUploadHeap.Get(), 0, 0, subResourceCount, textureData.data());
    commandList->ResourceBarrier(1, &barrier);

    // free
    for (auto ptr : m_imgPtrs)
    {
      stbi_image_free(ptr);
    }
  }
}
