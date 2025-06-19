#include "stdafx.h"
#include "DX12Texture.h"

#include "Core/DX12Interface.h"
#include "Core/DXApplicationHelper.h"
#include "Core/DX12FrameResource.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace Core
{
  DX12Texture::DX12Texture(std::vector<std::string> paths, unsigned mips)
    : m_imgPtrs()
    , m_metaData()
    , m_mipsLevels(mips)
  {
    for (const auto& path : paths)
    {
      MetaData metadata;
      m_imgPtrs.emplace_back(stbi_load(path.c_str(), &metadata.width, &metadata.height, &metadata.channels, 4));
      metadata.channels = 4; // force to 4
      m_metaData.push_back(metadata);
    }

    D3D12_RESOURCE_DESC textureDesc = {};
    textureDesc.MipLevels = m_mipsLevels;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.Width = m_metaData[0].width;
    textureDesc.Height = m_metaData[0].height;
    textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    textureDesc.DepthOrArraySize = m_imgPtrs.size();
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    
    auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

    ThrowIfFailed(DX12Interface::Get().GetDevice()->CreateCommittedResource(
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
    ThrowIfFailed(DX12Interface::Get().GetDevice()->CreateCommittedResource(
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

  void DX12Texture::GenerateMips(ID3D12GraphicsCommandList* commandList, CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandleSrc, CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandleOut)
  {
    //Transition from pixel shader resource to unordered access
    auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(
      m_texture.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    commandList->ResourceBarrier(1, &barrier1);

    SGenerateMipsCB generateMipsCB;
    generateMipsCB.SrcDimension = (m_texture->GetDesc().Height & 1) << 1 | (m_texture->GetDesc().Width & 1);
    generateMipsCB.SrcMipLevel = 0;
    generateMipsCB.NumMipLevels = m_mipsLevels;
    generateMipsCB.TexelSize.x = 1.0f / (float)m_texture->GetDesc().Width;
    generateMipsCB.TexelSize.y = 1.0f / (float)m_texture->GetDesc().Height;
    commandList->SetComputeRoot32BitConstants(0, sizeof(SGenerateMipsCB) / 4, &generateMipsCB, 0);
    commandList->SetComputeRootDescriptorTable(1, gpuHandleSrc);
    commandList->SetComputeRootDescriptorTable(2, gpuHandleOut);

    //Dispatch the compute shader with one thread per 8x8 pixels
    commandList->Dispatch(max(m_texture->GetDesc().Width / 8, 1u), max(m_texture->GetDesc().Height / 8, 1u), 1);

    barrier1 = CD3DX12_RESOURCE_BARRIER::UAV(m_texture.Get());
    //Wait for all accesses to the destination texture UAV to be finished before generating the next mipmap, as it will be the source texture for the next mipmap
    commandList->ResourceBarrier(1, &barrier1);

    barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(
      m_texture.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    commandList->ResourceBarrier(1, &barrier1);
  }
}
