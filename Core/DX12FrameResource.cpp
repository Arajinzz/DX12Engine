#include "stdafx.h"
#include "DX12FrameResource.h"

#include "Core/DXApplicationHelper.h"
#include "Core/WindowsApplication.h"
#include "Core/DX12SwapChain.h"

namespace Core
{
  DX12FrameResource::DX12FrameResource()
  {
    m_constantBuffer = std::make_unique<DX12ConstantBuffer>();
    m_texture = std::make_unique<DX12Texture>("textures\\brick.png");
    m_shader = std::make_unique<DX12Shader>(L"shaders.hlsl"); // shared between models

    m_shader->AddParameter(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX);
    m_shader->AddParameter(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, D3D12_SHADER_VISIBILITY_PIXEL);
    // this was in setup for model but since each model do this it will not work
    // this has to be rethinked
    m_shader->AddParameter(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX);
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

    m_constantBuffer->SetModel(XMMatrixTranspose(
      XMMatrixIdentity() * XMMatrixRotationZ(angle) * XMMatrixRotationY(angle * 2) * XMMatrixRotationX(angle) * XMMatrixTranslation(sin(angle), 0.0, 0.0)
    ));
    m_constantBuffer->SetView(XMMatrixTranspose(XMMatrixLookAtLH(
      XMVectorSet(0.0, 0.0, -10.0, 0.0), // camera position
      XMVectorSet(0.0, 0.0, 0.0, 0.0), // lookat position
      XMVectorSet(0.0, 1.0, 0.0, 0.0) // up vector
    )));

    RECT rect;
    GetClientRect(WindowsApplication::GetHwnd(), &rect);
    auto width = rect.right - rect.left;
    auto height = rect.bottom - rect.top;
    auto aspectRatio = static_cast<double>(width) / height;
    m_constantBuffer->SetProjection(XMMatrixTranspose(XMMatrixPerspectiveFovLH(45.0, aspectRatio, 1.0, 100.0)));
  }

  DX12FrameResource::~DX12FrameResource()
  {
  }
}