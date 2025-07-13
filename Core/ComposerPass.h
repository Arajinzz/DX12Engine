#pragma once

#include "Core/RenderPass.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace Core
{
  class ComposerPass : public RenderPass
  {
  public:
    ComposerPass();
    ~ComposerPass();

    virtual void Render(DX12Context* ctx) override;

  private:
    ComposerPass(const ComposerPass&) = delete;
    ComposerPass& operator=(const ComposerPass&) = delete;
  };
}
