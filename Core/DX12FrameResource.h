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
    void AddParameter(D3D12_DESCRIPTOR_RANGE_TYPE type, D3D12_SHADER_VISIBILITY visibility);
    void InitHeapDesc(DX12Heap* heapDesc, std::vector<DX12ConstantBuffer*> constantBuffers, std::vector<DX12Texture*> textures);
    // !!??
    DX12Shader* GetShader() { return m_shader.get(); }
    DX12Camera* GetCamera() { return m_camera.get(); }
    
    ~DX12FrameResource();

  private:
    std::unique_ptr<DX12Texture> m_texture;
    std::unique_ptr<DX12ConstantBuffer> m_constantBuffer;
    std::unique_ptr<DX12Shader> m_shader;
    std::unique_ptr<DX12Camera> m_camera;

  private:
    DX12FrameResource();
    DX12FrameResource(const DX12FrameResource&) = delete;
    DX12FrameResource& operator=(const DX12FrameResource&) = delete;
  };
}

