#pragma once

#include "Core/DX12Texture.h"
#include "Core/DX12Mesh.h"
#include "Core/DX12Shader.h"

namespace Core
{
  class DX12Skybox
  {
  public:
    DX12Skybox();
    ~DX12Skybox();
    
    void Setup(ID3D12GraphicsCommandList* commandList);
    void Update();
    
    DX12Mesh* GetMesh() { return m_cube.get(); }

  private:
    std::unique_ptr<DX12Mesh> m_cube;

  private:
    DX12Skybox(const DX12Skybox&) = delete;
    DX12Skybox& operator=(const DX12Skybox&) = delete;
  };
}

