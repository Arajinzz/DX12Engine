#pragma once

#include "Core/DX12Texture.h"
#include "Core/DX12Model.h"

namespace Textures
{
  class DX12Skybox : public DX12Model
  {
  public:
    DX12Skybox();
    ~DX12Skybox();
    
    virtual void LoadModel(const char* path) override;

  private:
    DX12Skybox(const DX12Skybox&) = delete;
    DX12Skybox& operator=(const DX12Skybox&) = delete;
  };
}

