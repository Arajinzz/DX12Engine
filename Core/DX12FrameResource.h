#pragma once

#include "Core/DX12CommandList.h"
#include "Core/DX12Shader.h"
#include "Core/DX12Camera.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

#define CreateFrameResource() DX12FrameResource::Instance()
#define FrameResource() DX12FrameResource::Instance()

namespace Core
{
  class DX12Texture;
  class DX12ConstantBuffer;
  class DX12Skybox;

  class DX12FrameResource
  {
  public:
    static DX12FrameResource& Instance()
    {
      static DX12FrameResource instance;
      return instance;
    }
    
    void CreateResources(DX12CommandList* commandList);
    void Update();
    DX12Camera* GetCamera() { return m_camera.get(); }
    ID3D12Resource* GetConstantBuffer() { return m_constantBuffer.Get(); }
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
    ComPtr<ID3D12Resource> m_constantBuffer;

    // camera and skybox
    std::unique_ptr<DX12Camera> m_camera;
    std::unique_ptr<DX12Skybox> m_skybox;

  private:
    DX12FrameResource();
    DX12FrameResource(const DX12FrameResource&) = delete;
    DX12FrameResource& operator=(const DX12FrameResource&) = delete;
  };
}

