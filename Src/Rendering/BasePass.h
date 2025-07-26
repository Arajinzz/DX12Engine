#pragma once

#include "Rendering/RenderPass.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace Rendering
{
  class BasePass : public RenderPass
  {
  public:
    BasePass();
    ~BasePass();

    virtual void Render(Graphics::DX12Context* ctx) override;

  private:
    BasePass(const BasePass&) = delete;
    BasePass& operator=(const BasePass&) = delete;
  };
}
