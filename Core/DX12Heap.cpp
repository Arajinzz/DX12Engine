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
    m_heap = DX12Interface::Get().CreateHeapDescriptor(m_type, static_cast<unsigned>(m_resources.size()));
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
    for (unsigned i = 0; i < m_resources.size(); ++i)
    {
      auto type = m_resourceTypes[i];
      auto resource = m_resources[i].Get();

      if (type == RENDERTARGET)
        DX12Interface::Get().CreateRenderTargetView(resource, m_heap.Get(), i);
      else if (type == DEPTH)
        DX12Interface::Get().CreateDepthStencilView(resource, m_heap.Get(), i);
      else if (type == TEXTURE || type == CUBEMAP)
        DX12Interface::Get().CreateShaderResourceView(resource, m_heap.Get(), i, type == CUBEMAP);
      else if (type == CONSTANTBUFFER)
        DX12Interface::Get().CreateConstantBufferView(resource, m_heap.Get(), i);
    }

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