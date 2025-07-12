#pragma once

#include "Core/RenderPass.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace Core
{
  class SkyboxPass : public RenderPass
  {
  public:
    SkyboxPass();
    ~SkyboxPass();

  private:
    SkyboxPass(const SkyboxPass&) = delete;
    SkyboxPass& operator=(const SkyboxPass&) = delete;

  };
}
