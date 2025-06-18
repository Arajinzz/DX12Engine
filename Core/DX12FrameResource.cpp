#include "stdafx.h"
#include "DX12FrameResource.h"

#include "Core/DXApplicationHelper.h"
#include "Core/WindowsApplication.h"
#include "Core/DX12SwapChain.h"
#include "Core/DX12Texture.h"
#include "Core/DX12Skybox.h"
#include "Core/DX12Interface.h"
#include "Core/DX12Shader.h"

namespace Core
{
  DX12FrameResource::DX12FrameResource()
    : m_constantBuffer(nullptr)
    , m_camera(nullptr)
    , m_pCbvDataBegin(nullptr)
    , m_constantBufferData()
    , m_swapChain(nullptr)
  {
    // create the swap chain, the swap chain will be bound to commandQueue inside DX12GraphicsContext
    m_swapChain = std::make_unique<DX12SwapChain>();
  }

  void DX12FrameResource::CreateResources(ID3D12GraphicsCommandList* commandList)
  {
    // create camera
    m_camera = std::make_unique<DX12Camera>(45.0, 0.5f, 10000.0f);
    
    // create the buffer
    m_constantBuffer = DX12Interface::Get().CreateConstantBuffer(sizeof(ConstantBufferData), D3D12_HEAP_TYPE_UPLOAD);
    // Map and initialize the constant buffer. We don't unmap this until the
    // app closes. Keeping things mapped for the lifetime of the resource is okay.
    CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
    ThrowIfFailed(m_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_pCbvDataBegin)));
    memcpy(m_pCbvDataBegin, &m_constantBufferData, sizeof(m_constantBufferData));

    m_skybox = std::make_unique<DX12Skybox>();
    m_skybox->GetMesh()->SetScale(XMFLOAT3(1.0f, 1.0f, 1.0f));
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
    m_constantBuffer.Reset();
  }

  DX12Camera* DX12FrameResource::GetCamera() { return m_camera.get(); }
  ID3D12Resource* DX12FrameResource::GetConstantBuffer() { return m_constantBuffer.Get(); }
  DX12Skybox* DX12FrameResource::GetSkybox() { return m_skybox.get(); }
  DX12SwapChain* DX12FrameResource::GetSwapChain() { return m_swapChain.get(); }

}