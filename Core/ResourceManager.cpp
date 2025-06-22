#include "stdafx.h"
#include "ResourceManager.h"

#include "Core/DX12Interface.h"
#include "Core/DXApplicationHelper.h"

namespace Core
{
  ResourceManager::ResourceManager()
    : m_resourcesHeap(nullptr)
    , m_nextFreeTex()
    , m_nextFreeMip()
    , m_nextFreeCB()
  {
    m_resourcesHeap = DX12Interface::Get().CreateHeapDescriptor(
      D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, RESOURCE_HEAP_SIZE);
    // populate free Index vector
    for (unsigned i = TEX_RANGE.begin; i <= TEX_RANGE.end; i++)
      m_nextFreeTex.push_back(i);
    for (unsigned i = MIPS_RANGE.begin; i <= MIPS_RANGE.end; i++)
      m_nextFreeMip.push_back(i);
    for (unsigned i = CB_RANGE.begin; i <= CB_RANGE.end; i++)
      m_nextFreeCB.push_back(i);
  }

  ResourceManager::~ResourceManager()
  {
    m_resourcesHeap.Reset();
  }

  D3D12_GPU_DESCRIPTOR_HANDLE ResourceManager::GetGpuHandle(unsigned index)
  {
    return CD3DX12_GPU_DESCRIPTOR_HANDLE(
      m_resourcesHeap->GetGPUDescriptorHandleForHeapStart(),
      index,
      DX12Interface::Get().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
  }

  D3D12_CPU_DESCRIPTOR_HANDLE ResourceManager::GetCpuHandle(unsigned index)
  {
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(
      m_resourcesHeap->GetCPUDescriptorHandleForHeapStart(),
      index,
      DX12Interface::Get().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
  }

  ResourceDescriptor ResourceManager::CreateConstantBufferResource(size_t size, D3D12_HEAP_TYPE type)
  {
    // check if space is available
    if (m_nextFreeCB.size() < 1)
      throw std::out_of_range("[CONSTANT_BUFFER] HEAP DESCRIPTOR HAVE NO SPACE LEFT!");

    ResourceDescriptor output;
    output.resource = DX12Interface::Get().CreateConstantBuffer(size, D3D12_HEAP_TYPE_UPLOAD);
    output.index = m_nextFreeCB.front();
    m_nextFreeCB.erase(m_nextFreeCB.begin());

    DX12Interface::Get().CreateConstantBufferView(output.resource.Get(), m_resourcesHeap.Get(), output.index);

    return output;
  }

  TextureDescriptor ResourceManager::CreateTextureResource(D3D12_RESOURCE_DESC& desc, bool isCubeMap, bool generateMips)
  {
    // check if space is available
    if (m_nextFreeTex.size() < 1)
      throw std::out_of_range("[TEXTURE] HEAP DESCRIPTOR HAVE NO SPACE LEFT!");

    // check if space is available
    if (m_nextFreeMip.size() < desc.MipLevels)
      throw std::out_of_range("[MIPS] HEAP DESCRIPTOR HAVE NO SPACE LEFT!");

    TextureDescriptor output;
    ComPtr<ID3D12Resource> texture;
    ComPtr<ID3D12Resource> upload;
    
    output.index = m_nextFreeTex.front();
    // index no longer free
    m_nextFreeTex.erase(m_nextFreeTex.begin());

    auto defaultProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

    // texture resource
    ThrowIfFailed(DX12Interface::Get().GetDevice()->CreateCommittedResource(
      &defaultProps,
      D3D12_HEAP_FLAG_NONE,
      &desc,
      D3D12_RESOURCE_STATE_COPY_DEST,
      nullptr,
      IID_PPV_ARGS(&texture)));

    // upload heap
    auto uploadDesc = CD3DX12_RESOURCE_DESC::Buffer(
      GetRequiredIntermediateSize(texture.Get(), 0, desc.DepthOrArraySize * desc.MipLevels));
    auto uploadProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    ThrowIfFailed(DX12Interface::Get().GetDevice()->CreateCommittedResource(
      &uploadProps,
      D3D12_HEAP_FLAG_NONE,
      &uploadDesc,
      D3D12_RESOURCE_STATE_GENERIC_READ,
      nullptr,
      IID_PPV_ARGS(&upload)));

    DX12Interface::Get().CreateShaderResourceView(texture.Get(), m_resourcesHeap.Get(), output.index, isCubeMap);

    // mips?
    if (generateMips && m_nextFreeMip.size() >= texture->GetDesc().MipLevels)
    {
      // root signature takes 4 UAV at once
      output.mipIndex = m_nextFreeMip.front();
      for (unsigned mip = 0; mip < texture->GetDesc().MipLevels; ++mip)
      {
        DX12Interface::Get().CreateUnorderedAccessView(texture.Get(), m_resourcesHeap.Get(), m_nextFreeMip.front(), mip);
        m_nextFreeMip.erase(m_nextFreeMip.begin());
      }
    }

    output.resource = texture;
    output.upload = upload;

    return output;
  }
}
