#include "stdafx.h"
#include "ResourceManager.h"

#include "Core/DX12Interface.h"
#include "Core/DXApplicationHelper.h"

namespace Core
{
  ResourceManager::ResourceManager()
    : m_texResources()
    , m_texHeap(nullptr)
  {
    m_texHeap = DX12Interface::Get().CreateHeapDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 65535);
  }

  ResourceManager::~ResourceManager()
  {
    m_texResources.clear();
    m_texHeap.Reset();
  }

  ResourceDescriptor ResourceManager::CreateTextureResource(
    D3D12_RESOURCE_DESC& desc, CD3DX12_HEAP_PROPERTIES& props, D3D12_RESOURCE_STATES state, bool createView)
  {
    ResourceDescriptor output;
    ComPtr<ID3D12Resource> texture;

    ThrowIfFailed(DX12Interface::Get().GetDevice()->CreateCommittedResource(
      &props,
      D3D12_HEAP_FLAG_NONE,
      &desc,
      state,
      nullptr,
      IID_PPV_ARGS(&texture)));

    if (createView)
    {
      DX12Interface::Get().CreateShaderResourceView(
        texture.Get(), m_texHeap.Get(), static_cast<unsigned>(m_texResources.size()), desc.DepthOrArraySize > 1 /* cube map */);

      output.cpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
        m_texHeap->GetCPUDescriptorHandleForHeapStart(),
        m_texResources.size(),
        DX12Interface::Get().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
      output.gpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(
        m_texHeap->GetGPUDescriptorHandleForHeapStart(),
        m_texResources.size(),
        DX12Interface::Get().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
    }

    output.resource = texture;
    m_texResources.push_back(output);

    return output;
  }

  ResourceDescriptor ResourceManager::CreateMipsForTexture(ResourceDescriptor texture)
  {
    ResourceDescriptor output;

    // create views
    for (unsigned mip = 0; mip < texture.resource->GetDesc().MipLevels; ++mip)
      DX12Interface::Get().CreateUnorderedAccessView(texture.resource.Get(), m_texHeap.Get(), mip + m_texResources.size(), mip);

    output.cpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
      m_texHeap->GetCPUDescriptorHandleForHeapStart(),
      m_texResources.size(),
      DX12Interface::Get().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
    output.gpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(
      m_texHeap->GetGPUDescriptorHandleForHeapStart(),
      m_texResources.size(),
      DX12Interface::Get().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

    // hack!!!
    for (unsigned mip = 0; mip < texture.resource->GetDesc().MipLevels; ++mip)
      m_texResources.push_back(output);

    return output;
  }
}
