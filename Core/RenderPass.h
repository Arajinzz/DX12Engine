#pragma once

#include "Core/PSOManager.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace Core
{
  class RenderPass
  {
  public:
    RenderPass();
    ~RenderPass();

    void SetPSO(ID3D12PipelineState* pso);

  private:
    ID3D12PipelineState* m_pso;

  };
}
