#pragma once

#include "Core\DX12Heap.h"
#include "Core\ResourceManager.h"

#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace Core
{
  class DX12Shader;
  class DX12Texture;

  class DX12Mesh
  {
  public:
    DX12Mesh(D3D12_CULL_MODE cullMode = D3D12_CULL_MODE_BACK, bool depthEnabled = true);
    ~DX12Mesh();

    virtual void Setup(ID3D12GraphicsCommandList* commandList, DX12Shader* shader);
    virtual void Draw(
      unsigned frameIndex, ResourceDescriptor cb, TextureDescriptor texture, DX12Shader* shader, ID3D12GraphicsCommandList* commandList);
    virtual void LoadMesh(const aiMesh* pMesh);
    unsigned GetTriangleCount() { return static_cast<unsigned>(m_indices.size() / 3); }

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

    D3D12_CULL_MODE m_cullMode;
    bool m_depthEnabled;
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
    DX12Mesh(const DX12Mesh&) = delete;
    DX12Mesh& operator=(const DX12Mesh&) = delete;

  };
}