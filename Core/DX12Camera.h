#pragma once

namespace Core
{
  class DX12Camera
  {
  public:
    DX12Camera();
    ~DX12Camera();

  private:
    DX12Camera(const DX12Camera&) = delete;
    DX12Camera& operator=(const DX12Camera&) = delete;
  };
}

