#pragma once

#include "Core/DX12Model.h"

namespace Core
{
  class DX12Shader;
  class DX12Texture;

  // Mesh can have multiple models
  class DX12Mesh
  {
  public:
    DX12Mesh();
    ~DX12Mesh();

    void SetupMesh(ID3D12GraphicsCommandList* commandList);
    void DrawMesh(unsigned frameIndex, ID3D12GraphicsCommandList* commandList);
    void LoadMesh(const char* path);
    // Workaround!!!
    void LoadMeshSkyboxSpecific(const char* path);
    void UpdateMesh();
    // workaround????
    DX12Heap* GetHeapDesc() { return m_descHeap.get(); }
    void SetTranslation(XMFLOAT3 translate) { m_translation = translate; };
    unsigned GetTriangleCount();

  private:
    std::vector<std::unique_ptr<DX12Model>> m_models;
    std::vector<std::unique_ptr<DX12Shader>> m_shaders; // for each model
    std::vector<std::unique_ptr<DX12Texture>> m_textures; // for each model
    std::unique_ptr<DX12Heap> m_descHeap;
    
    // constant buffer
    std::unique_ptr<DX12ConstantBuffer> m_constantBuffer;
    // for testing
    XMFLOAT3 m_translation;
    float m_angle;
    aiMatrix4x4 m_transformation;

  private:
    DX12Mesh(const DX12Mesh&) = delete;
    DX12Mesh& operator=(const DX12Mesh&) = delete;

  };
}

