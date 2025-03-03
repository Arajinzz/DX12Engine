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
      if (type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
        heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
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

  // TO REFACTOR
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
    } else if (m_type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
    {
      for (unsigned n = 0; n < m_descriptorCount; ++n)
      {
        auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(256);
        ThrowIfFailed(Device()->CreateCommittedResource(
          &heapProp,
          D3D12_HEAP_FLAG_NONE,
          &resDesc,
          D3D12_RESOURCE_STATE_GENERIC_READ,
          nullptr,
          IID_PPV_ARGS(&m_resources[n])));

        // Describe and create a constant buffer view.
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
        cbvDesc.BufferLocation = m_resources[n]->GetGPUVirtualAddress();
        cbvDesc.SizeInBytes = 256;
        auto handle = m_heap->GetCPUDescriptorHandleForHeapStart();
        Device()->CreateConstantBufferView(&cbvDesc, handle);
        Offset(1);
      }
    } else if (m_type == D3D12_DESCRIPTOR_HEAP_TYPE_DSV)
    {
      D3D12_RESOURCE_DESC depthStencilDesc = {};
      depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
      depthStencilDesc.Alignment = 0;
      depthStencilDesc.Width = 1280;  // Swap chain width
      depthStencilDesc.Height = 720; // Swap chain height
      depthStencilDesc.DepthOrArraySize = 1;
      depthStencilDesc.MipLevels = 1;
      depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
      depthStencilDesc.SampleDesc.Count = 1; // No multi-sampling
      depthStencilDesc.SampleDesc.Quality = 0;
      depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
      depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

      D3D12_CLEAR_VALUE clearValue = {};
      clearValue.Format = DXGI_FORMAT_D32_FLOAT;
      clearValue.DepthStencil.Depth = 1.0f;
      clearValue.DepthStencil.Stencil = 0;

      auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
      ThrowIfFailed(Device()->CreateCommittedResource(
        &heapProp,
        D3D12_HEAP_FLAG_NONE,
        &depthStencilDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &clearValue,
        IID_PPV_ARGS(&m_resources[0])));

      D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
      dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
      dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
      dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

      Device()->CreateDepthStencilView(m_resources[0].Get(), &dsvDesc, m_heap->GetCPUDescriptorHandleForHeapStart());
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