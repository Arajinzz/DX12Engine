#pragma once

#include "Core/DX12Camera.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

#define CreateFrameResource() DX12FrameResource::Instance()
#define FrameResource() DX12FrameResource::Instance()

namespace Core
{
  class DX12Texture;
  class DX12Skybox;
  class DX12SwapChain;
  class DX12Shader;

  class DX12FrameResource
  {
  public:
    static DX12FrameResource& Instance()
    {
      static DX12FrameResource instance;
      return instance;
    }
    
    void CreateResources(ID3D12GraphicsCommandList* commandList);
    void Update();

    DX12Camera* GetCamera();
    ID3D12Resource* GetConstantBuffer();
    DX12Skybox* GetSkybox();
    DX12SwapChain* GetSwapChain();

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

    // the swapchain
    std::unique_ptr<DX12SwapChain> m_swapChain;

  private:
    DX12FrameResource();
    DX12FrameResource(const DX12FrameResource&) = delete;
    DX12FrameResource& operator=(const DX12FrameResource&) = delete;
  };
}

