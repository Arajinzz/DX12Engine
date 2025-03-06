#include "stdafx.h"
#include "DX12ConstantBuffer.h"

#include "Core/DXApplicationHelper.h"
#include "Core/DX12Device.h"

namespace Core
{
  DX12ConstantBuffer::DX12ConstantBuffer(DX12Heap* heap)
  {
    auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(256);
    ThrowIfFailed(Device()->CreateCommittedResource(
      &heapProp,
      D3D12_HEAP_FLAG_NONE,
      &resDesc,
      D3D12_RESOURCE_STATE_GENERIC_READ,
      nullptr,
      IID_PPV_ARGS(&m_buffer)));

    heap->AddResource(m_buffer, CONSTANTBUFFER);
  }

  DX12ConstantBuffer::~DX12ConstantBuffer()
  {
  }
}