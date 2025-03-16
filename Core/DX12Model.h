#pragma once

#include "Core/DX12CommandList.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace Core
{
  class DX12Model
  {
  public:
    DX12Model();
    ~DX12Model();

    ID3D12GraphicsCommandList* GetBundle() { return m_bundle->Get(); }
    virtual void Draw(unsigned frameIndex);
    virtual void LoadModel(const char* path);
    virtual void Update();

  private:
    struct Vertex
    {
      XMFLOAT3 position;
      XMFLOAT4 color;
      XMFLOAT2 uv;
    };

    // data
    std::vector<Vertex> m_vertices;
    std::vector<uint16_t> m_indices;

    // bundle
    std::unique_ptr<DX12CommandList> m_bundle;
    // root signature and pso
    ComPtr<ID3D12RootSignature> m_rootSignature;
    ComPtr<ID3D12PipelineState> m_pipelineState;

    ComPtr<ID3D12Resource> m_vertexBuffer;
    ComPtr<ID3D12Resource> m_indexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
    D3D12_INDEX_BUFFER_VIEW m_indexBufferView;

  private:
    DX12Model(const DX12Model&) = delete;
    DX12Model& operator=(const DX12Model&) = delete;

  };
}

