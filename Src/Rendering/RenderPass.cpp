#include "stdafx.h"
#include "RenderPass.h"

namespace Rendering
{
  RenderPass::RenderPass()
    : m_pso()
    , m_rootSignature()
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

  void RenderPass::SetRootSignature(ID3D12RootSignature* rootSig)
  {
    // set the root signature
    m_rootSignature = rootSig;
  }
}
