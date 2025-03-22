#pragma once

#include "Core/DX12CommandList.h"

#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace Core
{
  class DX12Shader;
  class DX12Texture;
  class DX12ConstantBuffer;

  class DX12Model
  {
  public:
    DX12Model();
    ~DX12Model();

    ID3D12GraphicsCommandList* GetBundle() { return m_bundle->Get(); }
    virtual void Setup(ID3D12GraphicsCommandList* commandList, DX12Shader* shader);
    virtual void Draw(unsigned frameIndex, DX12Heap* heapDesc, DX12Shader* shader, unsigned texturePos);
    virtual void LoadModel(const aiMesh* pMesh);
    unsigned GetTriangleCount() { return m_indices.size() / 3; }

  private:
    void SetupVertexBuffer(ID3D12GraphicsCommandList* commandList);
    void SetupIndexBuffer(ID3D12GraphicsCommandList* commandList);

  private:
    struct Vertex
    {
      XMFLOAT3 position;
      XMFLOAT3 normal;
      XMFLOAT2 uv;
    };

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

  private:
    DX12Model(const DX12Model&) = delete;
    DX12Model& operator=(const DX12Model&) = delete;

  };
}

