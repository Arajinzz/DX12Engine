#include "stdafx.h"
#include "DX12FrameResource.h"

namespace Core
{
  DX12FrameResource::DX12FrameResource()
    : m_commandList(nullptr)
    , m_CbvSrvHeap(nullptr)
    , m_constantBufferData()
    , m_pCbvDataBegin(nullptr)
  {
    // create command list
    m_commandList = std::make_unique<DX12CommandList>();
    
    m_CbvSrvHeap = std::make_unique<DX12Heap>(2, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    m_CbvSrvHeap->CreateResources(sizeof(SceneConstantBuffer));

    m_commandList->Close();
  }

  DX12FrameResource::~DX12FrameResource()
  {
  }
}