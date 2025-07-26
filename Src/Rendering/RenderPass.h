#pragma once

#include "Core/PSOManager.h"
#include "Core/DX12Context.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace Rendering
{
  class RenderPass
  {
  public:
    RenderPass();
    ~RenderPass();

    virtual void Render(DX12Context* ctx) = 0;

    void SetPSO(ID3D12PipelineState* pso);
    void SetRootSignature(ID3D12RootSignature* rootSig);

  protected:
    ID3D12PipelineState* m_pso;
    ID3D12RootSignature* m_rootSignature;

  };
}
