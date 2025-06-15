#include "stdafx.h"
#include "DX12Heap.h"

#include "Core/DXApplicationHelper.h"
#include "Core/DX12Interface.h"
#include "Core/DX12SwapChain.h"

namespace Core
{
  DX12Heap::DX12Heap(D3D12_DESCRIPTOR_HEAP_TYPE type)
    : m_resources()
    , m_resourceTypes()
    , m_type(type)
  {
  }

  DX12Heap::~DX12Heap()
  {
    ResetResources();
    ResetHeap();
  }

  void DX12Heap::CreateHeap()
  {
    // Describe and create a render target view (RTV) descriptor heap.
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.NumDescriptors = m_resources.size();
    heapDesc.Type = m_type;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    if (m_type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
      heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(DX12Interface::Get().GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_heap)));
    m_descriptorSize = DX12Interface::Get().GetDevice()->GetDescriptorHandleIncrementSize(m_type);
    m_handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_heap->GetCPUDescriptorHandleForHeapStart());
  }

  void DX12Heap::ResetResources()
  {
    for (unsigned n = 0; n < m_resources.size(); ++n)
      m_resources[n].Reset();
    // clear resources but keep heap
    m_resources.clear();
    if (m_heap)
      m_handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_heap->GetCPUDescriptorHandleForHeapStart());
  }

  void DX12Heap::ResetHeap()
  {
    m_heap.Reset();
  }

  void DX12Heap::AddResource(ComPtr<ID3D12Resource> resource, ResourceType type)
  {
    m_resources.push_back(resource);
    m_resourceTypes.push_back(type);

    // maybe dynamic creation
    ResetHeap();
    CreateHeap();
    CreateViews();
  }

  void DX12Heap::CreateViews()
  {
    // workaround
    const unsigned mipLevels = 4;
    unsigned currentMipLevel = 0;
    for (unsigned i = 0; i < m_resources.size(); ++i)
    {
      auto type = m_resourceTypes[i];

      if (type == RENDERTARGET)
        DX12Interface::Get().GetDevice()->CreateRenderTargetView(m_resources[i].Get(), nullptr, m_handle);
      else if (type == DEPTH)
      {
        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

        DX12Interface::Get().GetDevice()->CreateDepthStencilView(m_resources[i].Get(), &dsvDesc, m_heap->GetCPUDescriptorHandleForHeapStart());
      }
      else if (type == TEXTURE)
      {
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = m_resources[i]->GetDesc().MipLevels;
        DX12Interface::Get().GetDevice()->CreateShaderResourceView(m_resources[i].Get(), &srvDesc, m_handle);
      }
      else if (type == UAV)
      {
        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
        uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        uavDesc.Texture2D.MipSlice = currentMipLevel;
        DX12Interface::Get().GetDevice()->CreateUnorderedAccessView(m_resources[i].Get(), nullptr, &uavDesc, m_handle);
        // Workaround
        currentMipLevel++;
        if (currentMipLevel >= mipLevels)
          currentMipLevel = 0;
      }
      else if (type == CUBEMAP)
      {
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
        srvDesc.Texture2D.MipLevels = 1;
        DX12Interface::Get().GetDevice()->CreateShaderResourceView(m_resources[i].Get(), &srvDesc, m_handle);
      } else if (type == CONSTANTBUFFER)
      {
        // Describe and create a constant buffer view
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
        cbvDesc.BufferLocation = m_resources[i]->GetGPUVirtualAddress();
        cbvDesc.SizeInBytes = 256;
        DX12Interface::Get().GetDevice()->CreateConstantBufferView(&cbvDesc, m_handle);
      }

      Offset(1);
    }

  }

  void DX12Heap::Offset(unsigned int padding)
  {
    m_handle.Offset(padding, m_descriptorSize);
  }

  CD3DX12_CPU_DESCRIPTOR_HANDLE DX12Heap::GetOffsetHandle(unsigned int Offset)
  {
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_heap->GetCPUDescriptorHandleForHeapStart(), Offset, m_descriptorSize);
  }

  CD3DX12_GPU_DESCRIPTOR_HANDLE DX12Heap::GetOffsetGPUHandle(unsigned int Offset)
  {
    return CD3DX12_GPU_DESCRIPTOR_HANDLE(m_heap->GetGPUDescriptorHandleForHeapStart(), Offset, m_descriptorSize);
  }
}