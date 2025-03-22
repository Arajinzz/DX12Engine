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
    DX12ConstantBuffer* GetConstantBuffer() { return m_constantBuffer.get(); }

    ~DX12FrameResource();

  private:
    std::unique_ptr<DX12ConstantBuffer> m_constantBuffer;
    std::unique_ptr<DX12Camera> m_camera;
    std::unique_ptr<DX12Skybox> m_skybox;

  private:
    DX12FrameResource();
    DX12FrameResource(const DX12FrameResource&) = delete;
    DX12FrameResource& operator=(const DX12FrameResource&) = delete;
  };
}

