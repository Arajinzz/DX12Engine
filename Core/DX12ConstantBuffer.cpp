#include "stdafx.h"
#include "DX12ConstantBuffer.h"

#include "Core/DXApplicationHelper.h"
#include "Core/DX12Interface.h"
#include "Core/DX12FrameResource.h"

namespace Core
{
  DX12ConstantBuffer::DX12ConstantBuffer()
    : m_constantBufferData()
    , m_pCbvDataBegin(nullptr)
  {
    auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(256);
    ThrowIfFailed(DX12Interface::Get().GetDevice()->CreateCommittedResource(
      &heapProp,
      D3D12_HEAP_FLAG_NONE,
      &resDesc,
      D3D12_RESOURCE_STATE_GENERIC_READ,
      nullptr,
      IID_PPV_ARGS(&m_buffer)));

    // Map and initialize the constant buffer. We don't unmap this until the
    // app closes. Keeping things mapped for the lifetime of the resource is okay.
    CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
    ThrowIfFailed(m_buffer->Map(0, &readRange, reinterpret_cast<void**>(&m_pCbvDataBegin)));
    memcpy(m_pCbvDataBegin, &m_constantBufferData, sizeof(m_constantBufferData));
  }

  DX12ConstantBuffer::~DX12ConstantBuffer()
  {
  }

  void DX12ConstantBuffer::SetModel(XMMATRIX model)
  {
    m_constantBufferData.model = model;
    memcpy(m_pCbvDataBegin, &m_constantBufferData, sizeof(m_constantBufferData));
  }

  void DX12ConstantBuffer::SetView(XMMATRIX view)
  {
    m_constantBufferData.view = view;
    memcpy(m_pCbvDataBegin, &m_constantBufferData, sizeof(m_constantBufferData));
  }

  void DX12ConstantBuffer::SetProjection(XMMATRIX proj)
  {
    m_constantBufferData.projection = proj;
    memcpy(m_pCbvDataBegin, &m_constantBufferData, sizeof(m_constantBufferData));
  }
}