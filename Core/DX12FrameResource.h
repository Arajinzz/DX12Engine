#pragma once

#include "Core\DX12Camera.h"
#include "Core\ResourceManager.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

#define CreateFrameResource() DX12FrameResource::Instance()
#define FrameResource() DX12FrameResource::Instance()

namespace Core
{
  class DX12Texture;
  class DX12Skybox;

  class DX12FrameResource
  {
  public:
    static DX12FrameResource& Instance()
    {
      static DX12FrameResource instance;
      return instance;
    }

    void CreateResources();
    void Update();
    const ResourceDescriptor* GetConstantBuffer() { return m_constantBuffer.get(); }
    DX12Camera* GetCamera() { return m_camera.get(); }
    DX12Skybox* GetSkybox() { return m_skybox.get(); }

    ~DX12FrameResource();

  private:

    // temporary
    struct ConstantBufferData
    {
      XMMATRIX model;
      XMMATRIX view;
      XMMATRIX projection;
      float padding[16]; // Padding so the constant buffer is 256-byte aligned.
    };
    // data
    ConstantBufferData m_constantBufferData;
    UINT8* m_pCbvDataBegin;
    // CB resource
    std::unique_ptr<ResourceDescriptor> m_constantBuffer;

    // camera and skybox
    std::unique_ptr<DX12Camera> m_camera;
    std::unique_ptr<DX12Skybox> m_skybox;

  private:
    DX12FrameResource();
    DX12FrameResource(const DX12FrameResource&) = delete;
    DX12FrameResource& operator=(const DX12FrameResource&) = delete;
  };
}

