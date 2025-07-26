#include "stdafx.h"
#include "BasePass.h"

#include "Core/SceneGraph.h"

namespace Rendering
{
  BasePass::BasePass()
    : RenderPass()
  {
  }

  BasePass::~BasePass()
  {
  }

  void BasePass::Render(DX12Context* ctx)
  {
    // draw meshes
    for (auto model : SceneGraph::Instance().GetModels())
      ctx->Draw(model, m_pso, m_rootSignature);
  }
}
