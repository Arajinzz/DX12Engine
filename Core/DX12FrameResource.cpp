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
    , m_texture(nullptr)
    , m_shader(nullptr)
  {
  }

  void DX12FrameResource::CreateResources(DX12CommandList* commandList)
  {
    // Shader first because constant buffers and textures modify the root signature of this shader
    m_shader = std::make_unique<DX12Shader>(L"shaders.hlsl"); // shared between models
    
    // add slots
    m_shader->AddParameter(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX);
    m_shader->AddParameter(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX);
    m_shader->AddParameter(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, D3D12_SHADER_VISIBILITY_PIXEL);

    m_constantBuffer = std::make_unique<DX12ConstantBuffer>();
    m_texture = std::make_unique<DX12Texture>("textures\\brick.png"); // shared between models
    m_texture->CopyToGPU(commandList->Get());
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
    m_constantBuffer->SetProjection(XMMatrixTranspose(XMMatrixPerspectiveFovLH(45.0, aspectRatio, 1.0, 1000.0)));
  }

  void DX12FrameResource::AddParameter(D3D12_DESCRIPTOR_RANGE_TYPE type, D3D12_SHADER_VISIBILITY visibility)
  {
    m_shader->AddParameter(type, visibility);
  }

  void DX12FrameResource::InitHeapDesc(DX12Heap* heapDesc, std::vector<DX12ConstantBuffer*> constantBuffers, std::vector<DX12Texture*> textures)
  {
    // always Globals first
    heapDesc->AddResource(m_constantBuffer->GetResource(), CONSTANTBUFFER);
    for (auto constantBuffer : constantBuffers)
      heapDesc->AddResource(constantBuffer->GetResource(), CONSTANTBUFFER);

    heapDesc->AddResource(m_texture->GetResource(), TEXTURE);
    for (auto texture : textures)
      heapDesc->AddResource(texture->GetResource(), TEXTURE);
  }

  DX12FrameResource::~DX12FrameResource()
  {
    m_shader.reset();
    m_constantBuffer.reset();
    m_texture.reset();
  }
}