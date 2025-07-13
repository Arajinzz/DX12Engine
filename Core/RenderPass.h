#pragma once

#include "Core/PSOManager.h"
#include "Core/DX12Context.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace Core
{
  class RenderPass
  {
  public:
    RenderPass();
    ~RenderPass();

    virtual void Render(DX12Context* ctx) = 0;

    void SetPSO(ID3D12PipelineState* pso);

  private:
    ID3D12PipelineState* m_pso;

  };
}
