#pragma once

#include "Rendering/RenderPass.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace Rendering
{
  class SkyboxPass : public RenderPass
  {
  public:
    SkyboxPass();
    ~SkyboxPass();

    virtual void Render(Graphics::DX12Context* ctx) override;

  private:
    SkyboxPass(const SkyboxPass&) = delete;
    SkyboxPass& operator=(const SkyboxPass&) = delete;

  };
}
