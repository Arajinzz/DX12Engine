#include "stdafx.h"
#include "DX12FrameResource.h"

#include "Core/DXApplicationHelper.h"
#include "Core/WindowsApplication.h"
#include "Core/DX12SwapChain.h"
#include "Core/DX12Texture.h"
#include "Core/DX12Skybox.h"
#include "Core/DX12Interface.h"

namespace Core
{
  DX12FrameResource::DX12FrameResource()
    : m_constantBuffer(nullptr)
    , m_camera(nullptr)
    , m_pCbvDataBegin(nullptr)
    , m_constantBufferData()
  {
  }

  void DX12FrameResource::CreateResources(ID3D12GraphicsCommandList* commandList)
  {
    // create camera
    m_camera = std::make_unique<DX12Camera>(45.0, 0.5f, 10000.0f);
    
    // create the buffer
    m_constantBuffer = ResourceManager::Instance().CreateConstantBufferResource(
      sizeof(ConstantBufferData), D3D12_HEAP_TYPE_UPLOAD);
    // Map and initialize the constant buffer. We don't unmap this until the
    // app closes. Keeping things mapped for the lifetime of the resource is okay.
    CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
    ThrowIfFailed(m_constantBuffer->resource.Get()->Map(0, &readRange, reinterpret_cast<void**>(&m_pCbvDataBegin)));
    memcpy(m_pCbvDataBegin, &m_constantBufferData, sizeof(m_constantBufferData));

    m_skybox = std::make_unique<DX12Skybox>();
    m_skybox->GetModel()->SetScale(XMFLOAT3(1.0f, 1.0f, 1.0f));
  }

  void DX12FrameResource::Update()
  {
    m_camera->Update();
    m_skybox->Update();

    m_constantBufferData.view = m_camera->GetView();
    m_constantBufferData.projection = m_camera->GetProjection();
    memcpy(m_pCbvDataBegin, &m_constantBufferData, sizeof(m_constantBufferData));
  }

  DX12FrameResource::~DX12FrameResource()
  {
  }
}