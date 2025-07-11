#pragma once

#include "Core\ResourceManager.h"

#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace Core
{
  class DX12Texture;

  class DX12Mesh
  {
  public:
    DX12Mesh(D3D12_CULL_MODE cullMode = D3D12_CULL_MODE_BACK, bool depthEnabled = true);
    ~DX12Mesh();

    virtual void Setup(ID3D12GraphicsCommandList* commandList);
    virtual void Draw(
      unsigned frameIndex, ResourceDescriptor* cb, TextureDescriptor* texture, bool cubeMap, ID3D12GraphicsCommandList* commandList);
    virtual void LoadMesh(const aiMesh* pMesh, const aiMatrix4x4& transform);
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

namespace Core
{
  class DX12Texture;

  // Mesh can have multiple models
  class DX12Model
  {
  public:
    DX12Model();
    ~DX12Model();

    void SetupModel(ID3D12GraphicsCommandList* commandList);
    void DrawModel(unsigned frameIndex, ID3D12GraphicsCommandList* commandList);
    void LoadModel(const char* path);
    // Workaround!!!
    void LoadModelSkyboxSpecific(const char* path);
    void UpdateModel();
    void SetTranslation(XMFLOAT3 translate) { m_translation = translate; };
    void SetScale(XMFLOAT3 scale) { m_scale = scale; }
    unsigned GetTriangleCount();

  private:
    void ProcessNode(const aiNode* node, const aiScene* scene, const aiMatrix4x4& parentTransform);

  private:
    std::vector<std::unique_ptr<DX12Mesh>> m_meshes;
    std::vector<std::shared_ptr<DX12Texture>> m_textures; // for each model
    bool staticMesh = true;
    bool isModelSet = false;

    // temporary
    struct ConstantBufferData
    {
      XMMATRIX model;
      XMMATRIX view;
      XMMATRIX projection;
      float padding[16]; // Padding so the constant buffer is 256-byte aligned.
    };
    // data
    ConstantBufferData m_constantBufferData;
    UINT8* m_pCbvDataBegin;
    // constant buffer
    std::unique_ptr<ResourceDescriptor> m_constantBuffer;

    // for testing
    XMFLOAT3 m_translation;
    XMFLOAT3 m_scale;
    float m_angle;
    // workaround
    bool m_isCubeMap;

  private:
    DX12Model(const DX12Model&) = delete;
    DX12Model& operator=(const DX12Model&) = delete;

  };
}

