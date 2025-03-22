#pragma once

#include "Core/DX12Texture.h"
#include "Core/DX12Mesh.h"

namespace Core
{
  class DX12Skybox
  {
  public:
    DX12Skybox();
    ~DX12Skybox();
    
    void Draw();
    void Update();

  private:
    std::unique_ptr<DX12Mesh> m_cube;
    std::unique_ptr<DX12Texture> m_cubeMap;

  private:
    DX12Skybox(const DX12Skybox&) = delete;
    DX12Skybox& operator=(const DX12Skybox&) = delete;
  };
}

