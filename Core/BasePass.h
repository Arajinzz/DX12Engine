#pragma once

#include "Core/RenderPass.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace Core
{
  class BasePass : public RenderPass
  {
  public:
    BasePass();
    ~BasePass();

    virtual void Render(DX12Context* ctx) override;

  private:
    BasePass(const BasePass&) = delete;
    BasePass& operator=(const BasePass&) = delete;
  };
}
