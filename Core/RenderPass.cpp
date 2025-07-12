#include "stdafx.h"
#include "RenderPass.h"

namespace Core
{
  RenderPass::RenderPass()
    : m_pso()
  {
  }

  RenderPass::~RenderPass()
  {
  }

  void RenderPass::SetPSO(ID3D12PipelineState* pso)
  {
    // set the pso
    m_pso = pso;
  }
}
