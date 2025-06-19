#pragma once

#include "Core/DX12Texture.h"
#include "Core/DX12Model.h"
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
    
    DX12Model* GetModel() { return m_cube.get(); }

  private:
    std::unique_ptr<DX12Model> m_cube;

  private:
    DX12Skybox(const DX12Skybox&) = delete;
    DX12Skybox& operator=(const DX12Skybox&) = delete;
  };
}

