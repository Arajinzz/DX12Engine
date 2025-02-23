#include "stdafx.h"
#include "DX12Heap.h"

#include "Core/DXApplicationHelper.h"
#include "Core/DX12Device.h"

namespace Core
{
  DX12Heap::DX12Heap(unsigned int numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type, DX12SwapChain* swapChain)
    : m_descriptorCount(numDescriptors)
    , m_resources(numDescriptors)
    , m_swapChain(swapChain)
    , m_type(type)
  {
    {
      // Describe and create a render target view (RTV) descriptor heap.
      D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
      heapDesc.NumDescriptors = numDescriptors;
      heapDesc.Type = type;
      heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
      ThrowIfFailed(Device()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_heap)));
      m_descriptorSize = Device()->GetDescriptorHandleIncrementSize(type);
    }

    m_handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_heap->GetCPUDescriptorHandleForHeapStart());
  }

  DX12Heap::~DX12Heap()
  {
    for (unsigned n = 0; n < m_descriptorCount; ++n)
      m_resources[n].Reset();
    m_heap.Reset();
  }

  void DX12Heap::CreateResources()
  {
    // other types not supported for now
    if (m_type == D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
    {
      for (unsigned n = 0; n < m_descriptorCount; ++n)
      {
        m_swapChain->GetBuffer(n, &m_resources[n]);
        Device()->CreateRenderTargetView(m_resources[n].Get(), nullptr, m_handle);
        Offset(1);
      }
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
}