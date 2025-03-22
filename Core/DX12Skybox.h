#pragma once

namespace Core
{
  class DX12Skybox
  {
  public:
    DX12Skybox();
    ~DX12Skybox();

  private:
    DX12Skybox(const DX12Skybox&) = delete;
    DX12Skybox& operator=(const DX12Skybox&) = delete;
  };
}

