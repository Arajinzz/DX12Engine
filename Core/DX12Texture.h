#pragma once

namespace Core
{
  class DX12Texture
  {
  public:
    DX12Texture();
    ~DX12Texture();

  private:
    DX12Texture(const DX12Texture&) = delete;
    DX12Texture& operator=(const DX12Texture&) = delete;
  };
}

