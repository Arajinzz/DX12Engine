#pragma once

#include "Core/RenderPass.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

// this will create a quad and map a texture to the screen
namespace Core
{
  class ComposerPass : public RenderPass
  {
  public:
    ComposerPass();
    ~ComposerPass();

    virtual void Render(DX12Context* ctx) override;

  private:
    // setup functions
    void SetupVertexBuffer(ID3D12GraphicsCommandList* commandList);
    void SetupIndexBuffer(ID3D12GraphicsCommandList* commandList);

  private:
    struct Vertex
    {
      XMFLOAT3 position;
      XMFLOAT3 normal;
      XMFLOAT2 uv;
    };
    Vertex m_vertices[4]; // for a quad
    uint16_t m_indices[6]; // 2 triangles needed for a quad
    // vertex buffer
    ComPtr<ID3D12Resource> m_vertexBuffer;
    ComPtr<ID3D12Resource> m_vertexBufferUploadHeap;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
    // index buffer
    ComPtr<ID3D12Resource> m_indexBuffer;
    ComPtr<ID3D12Resource> m_indexBufferUploadHeap;
    D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
    // did we setup?
    bool m_quadReady;

  private:
    ComposerPass(const ComposerPass&) = delete;
    ComposerPass& operator=(const ComposerPass&) = delete;
  };
}
