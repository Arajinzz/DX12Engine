#include "stdafx.h"
#include "DX12FrameResource.h"

#include "Core/DXApplicationHelper.h"
#include "Core/WindowsApplication.h"
#include "Core/DX12SwapChain.h"

namespace Core
{
  DX12FrameResource::DX12FrameResource()
    : m_commandList(nullptr)
    , m_CbvSrvHeap(nullptr)
    , m_constantBufferData()
    , m_pCbvDataBegin(nullptr)
  {
    m_CbvSrvHeap = std::make_unique<DX12Heap>(2, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    m_constantBuffer = std::make_unique<DX12ConstantBuffer>(m_CbvSrvHeap.get());
    m_texture = std::make_unique<DX12Texture>(m_CbvSrvHeap.get());

    // create command list
    m_commandList = std::make_unique<DX12CommandList>();
    m_CbvSrvHeap->CreateResources(); // swapchain not needed
    m_commandList->Close();

    // Map and initialize the constant buffer. We don't unmap this until the
    // app closes. Keeping things mapped for the lifetime of the resource is okay.
    CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
    ThrowIfFailed(m_CbvSrvHeap->GetResource(0)->Map(0, &readRange, reinterpret_cast<void**>(&m_pCbvDataBegin)));
    memcpy(m_pCbvDataBegin, &m_constantBufferData, sizeof(m_constantBufferData));
  }

  void DX12FrameResource::Init(DX12CommandList* commandList)
  {
    m_texture->Init(commandList->Get());
  }

  void DX12FrameResource::Update()
  {
    auto static angle = 0.0;
    angle += 3 * WindowsApplication::deltaTime;
    if (angle > 360.0)
      angle = 0.0;

    m_constantBufferData.model = XMMatrixTranspose(
      XMMatrixIdentity() * XMMatrixRotationZ(angle) * XMMatrixRotationY(angle * 2) * XMMatrixRotationX(angle) * XMMatrixTranslation(sin(angle), 0.0, 0.0)
    );
    m_constantBufferData.view = XMMatrixTranspose(XMMatrixLookAtLH(
      XMVectorSet(0.0, 0.0, -3.0, 0.0), // camera position
      XMVectorSet(0.0, 0.0, 0.0, 0.0), // lookat position
      XMVectorSet(0.0, 1.0, 0.0, 0.0) // up vector
    ));
    ComPtr<ID3D12Resource> renderTarget;
    SwapChain().GetBuffer(0, &renderTarget);
    auto aspectRatio = static_cast<double>(renderTarget->GetDesc().Width) / renderTarget->GetDesc().Height;
    m_constantBufferData.projection = XMMatrixTranspose(XMMatrixPerspectiveFovLH(45.0, aspectRatio, 1.0, 100.0));
    memcpy(m_pCbvDataBegin, &m_constantBufferData, sizeof(m_constantBufferData));
  }

  DX12FrameResource::~DX12FrameResource()
  {
  }
}