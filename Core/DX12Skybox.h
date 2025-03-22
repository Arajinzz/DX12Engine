#pragma once

#include "Core/DX12Texture.h"

namespace Core
{
  class DX12Skybox
  {
  public:
    DX12Skybox();
    ~DX12Skybox();

  private:
    std::unique_ptr<DX12Texture> m_cubeMap;

  private:
    DX12Skybox(const DX12Skybox&) = delete;
    DX12Skybox& operator=(const DX12Skybox&) = delete;
  };
}

