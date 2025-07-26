#include "stdafx.h"
#include "SkyboxPass.h"

#include "Scene/DX12Skybox.h"
#include "Scene/SceneGraph.h"

namespace Rendering
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
