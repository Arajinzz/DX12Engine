#include "stdafx.h"
#include "DX12Texture.h"

#include "Core/DX12Interface.h"
#include "Core/DXApplicationHelper.h"
#include "Core/DX12FrameResource.h"
#include "Core/PSOManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace Core
{
  DX12Texture::DX12Texture(std::vector<std::string> paths, unsigned mips)
    : m_imgPtrs()
    , m_metaData()
    , m_mipsLevels(mips)
    , m_texture()
    , m_uploaded(false)
    , m_mipsGenerated(false)
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
    textureDesc.DepthOrArraySize = static_cast<unsigned>(m_imgPtrs.size());
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

    // don't generate mips for skybox
    m_texture = ResourceManager::Instance().CreateTextureResource(textureDesc, m_imgPtrs.size() > 1, m_imgPtrs.size() == 1);
  }

  DX12Texture::~DX12Texture()
  {
    m_texture.reset();
  }

  void DX12Texture::CopyToGPU(ID3D12GraphicsCommandList* commandList)
  {
    if (m_uploaded)
      return;

    m_uploaded = true;
    // Copy data to the intermediate upload heap and then schedule 
    // a copy from the upload heap to the diffuse texture.
    std::vector<D3D12_SUBRESOURCE_DATA> textureData(m_imgPtrs.size());

    for (unsigned i = 0; i < m_imgPtrs.size(); ++i)
    {
      textureData[i].pData = m_imgPtrs[i];
      textureData[i].RowPitch = m_metaData[i].width * m_metaData[i].channels; // width * pixelSize
      textureData[i].SlicePitch = textureData[i].RowPitch * m_metaData[i].height; // rowpitch * height
    }

    const UINT subResourceCount = static_cast<unsigned>(1 * m_metaData.size());
    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
      m_texture->resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    UpdateSubresources(commandList, m_texture->resource.Get(), m_texture->upload.Get(), 0, 0, subResourceCount, textureData.data());
    commandList->ResourceBarrier(1, &barrier);

    // free
    for (auto ptr : m_imgPtrs)
      stbi_image_free(ptr);
  }

  void DX12Texture::GenerateMips(ID3D12GraphicsCommandList* commandList)
  {
    if (m_mipsGenerated)
      return;

    m_mipsGenerated = true;

    //Set root signature, pso and descriptor heap
    commandList->SetComputeRootSignature(PSOManager::Instance().GetRootSignature("MipsCompute"));
    commandList->SetPipelineState(PSOManager::Instance().GetPSO("MipsCompute"));

    // heap created
    ID3D12DescriptorHeap* ppHeaps[] = { ResourceManager::Instance().GetResourcesHeap() };
    commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

    //Transition from pixel shader resource to unordered access
    auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(
      m_texture->resource.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    commandList->ResourceBarrier(1, &barrier1);

    SGenerateMipsCB generateMipsCB;
    generateMipsCB.SrcDimension = (m_texture->resource->GetDesc().Height & 1) << 1 | (m_texture->resource->GetDesc().Width & 1);
    generateMipsCB.SrcMipLevel = 0;
    generateMipsCB.NumMipLevels = m_mipsLevels;
    generateMipsCB.TexelSize.x = 1.0f / (float)m_texture->resource->GetDesc().Width;
    generateMipsCB.TexelSize.y = 1.0f / (float)m_texture->resource->GetDesc().Height;
    commandList->SetComputeRoot32BitConstants(0, sizeof(SGenerateMipsCB) / 4, &generateMipsCB, 0);
    commandList->SetComputeRootDescriptorTable(1, ResourceManager::Instance().GetResourceGpuHandle(m_texture->index));
    commandList->SetComputeRootDescriptorTable(2, ResourceManager::Instance().GetResourceGpuHandle(m_texture->mipIndex));

    //Dispatch the compute shader with one thread per 8x8 pixels
    commandList->Dispatch(
      max(static_cast<unsigned>(m_texture->resource->GetDesc().Width / 8u), 1u), max(static_cast<unsigned>(m_texture->resource->GetDesc().Height / 8u), 1u), 1);

    barrier1 = CD3DX12_RESOURCE_BARRIER::UAV(m_texture->resource.Get());
    //Wait for all accesses to the destination texture UAV to be finished before generating the next mipmap, as it will be the source texture for the next mipmap
    commandList->ResourceBarrier(1, &barrier1);

    barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(
      m_texture->resource.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    commandList->ResourceBarrier(1, &barrier1);
  }
}
