#include "stdafx.h"
#include "SkyboxPass.h"

#include "Core/DX12Skybox.h"
#include "Core/SceneGraph.h"

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
    ctx->Draw(SceneGraph::Instance().GetSkybox(), m_pso, m_rootSignature);
  }
}
