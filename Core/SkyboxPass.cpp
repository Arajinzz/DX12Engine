#include "stdafx.h"
#include "SkyboxPass.h"

#include "Core/DX12FrameResource.h"
#include "Core/DX12Skybox.h"

namespace Core
{
  SkyboxPass::SkyboxPass()
    : RenderPass()
  {
  }

  SkyboxPass::~SkyboxPass()
  {
  }

  void SkyboxPass::Render(DX12Context* ctx)
  {
    // draw skybox first
    ctx->Draw(FrameResource().GetSkybox()->GetModel());
  }
}
