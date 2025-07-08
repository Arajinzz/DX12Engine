#pragma once

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace Core
{
  class BasePass
  {
  public:
    BasePass();
    ~BasePass();

  private:
    BasePass(const BasePass&) = delete;
    BasePass& operator=(const BasePass&) = delete;
  };
}
