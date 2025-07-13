#include "stdafx.h"
#include "BasePass.h"

namespace Core
{
  BasePass::BasePass()
    : RenderPass()
  {
  }

  BasePass::~BasePass()
  {
  }

  void BasePass::Render(ID3D12CommandList* cmdList)
  {
  }
}
