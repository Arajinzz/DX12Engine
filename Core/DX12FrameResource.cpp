#include "stdafx.h"
#include "DX12FrameResource.h"

#include "Core/DXApplicationHelper.h"
#include "Core/WindowsApplication.h"
#include "Core/DX12SwapChain.h"
#include "Core/DX12Texture.h"
#include "Core/DX12ConstantBuffer.h"

namespace Core
{
  DX12FrameResource::DX12FrameResource()
    : m_constantBuffer(nullptr)
    , m_camera(nullptr)
  {
  }

  void DX12FrameResource::CreateResources(DX12CommandList* commandList)
  {
    // create camera
    m_camera = std::make_unique<DX12Camera>(45.0, 0.1f, 100000.0f);
    m_constantBuffer = std::make_unique<DX12ConstantBuffer>();
  }

  void DX12FrameResource::Update()
  {
    m_camera->Update();
    m_constantBuffer->SetView(m_camera->GetView());
    m_constantBuffer->SetProjection(m_camera->GetProjection());
  }

  DX12FrameResource::~DX12FrameResource()
  {
    m_constantBuffer.reset();
  }
}