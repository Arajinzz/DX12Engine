#pragma once

#include "Core/DX12CommandList.h"
#include "Core/DX12Heap.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace Core
{
  class DX12Cube
  {
  public:
    DX12Cube(unsigned viewportWidth, unsigned viewportHeight, float padding = 0.0);
    ~DX12Cube();

    ID3D12GraphicsCommandList* GetBundle() { return m_bundle->Get(); }

    void Draw(unsigned frameIndex);
    void Update();
    ID3D12PipelineState* GetPSO() { return m_pipelineState.Get(); }

  private:
    struct Vertex
    {
      XMFLOAT3 position;
      XMFLOAT4 color;
      XMFLOAT2 uv;
    };

    std::unique_ptr<DX12CommandList> m_bundle;

    // root signature and pso
    ComPtr<ID3D12RootSignature> m_rootSignature;
    ComPtr<ID3D12PipelineState> m_pipelineState;

    ComPtr<ID3D12Resource> m_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
    ComPtr<ID3D12Resource> m_indexBuffer;
    D3D12_INDEX_BUFFER_VIEW m_indexBufferView;

  private:
    DX12Cube(const DX12Cube&) = delete;
    DX12Cube& operator=(const DX12Cube&) = delete;

  };
}

