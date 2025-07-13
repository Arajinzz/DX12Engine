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

    virtual void Render(ID3D12CommandList* cmdList) = 0;

    void SetPSO(ID3D12PipelineState* pso);

  private:
    ID3D12PipelineState* m_pso;

  };
}
