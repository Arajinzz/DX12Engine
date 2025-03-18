#pragma once

#include "Core/DX12CommandList.h"
#include "Core/DX12ConstantBuffer.h"

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
    virtual void Setup(ID3D12GraphicsCommandList* commandList);
    virtual void Draw(unsigned frameIndex);
    virtual void LoadModel(const char* path);
    virtual void Update();
    // workaround!!!
    DX12Heap* GetHeapDesc() { return m_descHeap.get(); }

    unsigned GetTriangleCount() { return m_indices.size() / 3; }

    void SetTranslation(XMFLOAT3 translate) { m_translation = translate; };

  private:
    void SetupVertexBuffer(ID3D12GraphicsCommandList* commandList);
    void SetupIndexBuffer(ID3D12GraphicsCommandList* commandList);

  private:
    struct Vertex
    {
      XMFLOAT3 position;
      XMFLOAT4 color;
      XMFLOAT2 uv;
    };

    // for testing
    XMFLOAT3 m_translation;
    float m_angle;

    // data
    std::vector<Vertex> m_vertices;
    std::vector<uint16_t> m_indices;

    // bundle
    std::unique_ptr<DX12CommandList> m_bundle;
    // pso
    ComPtr<ID3D12PipelineState> m_pipelineState;

    // buffers
    ComPtr<ID3D12Resource> m_vertexBuffer;
    ComPtr<ID3D12Resource> m_vertexBufferUploadHeap;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

    ComPtr<ID3D12Resource> m_indexBuffer;
    ComPtr<ID3D12Resource> m_indexBufferUploadHeap;
    D3D12_INDEX_BUFFER_VIEW m_indexBufferView;

    // descriptor heap
    std::unique_ptr<DX12Heap> m_descHeap;
    // constant buffer
    std::unique_ptr<DX12ConstantBuffer> m_constantBuffer;

  private:
    DX12Model(const DX12Model&) = delete;
    DX12Model& operator=(const DX12Model&) = delete;

  };
}

