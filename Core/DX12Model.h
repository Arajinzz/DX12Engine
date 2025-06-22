#pragma once

#include "Core/DX12Mesh.h"
#include "Core/DX12Heap.h"

namespace Core
{
  class DX12Shader;
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

    DX12Texture* GetTexture(unsigned index) { return m_textures[index].get(); }
    unsigned GetTexturesCount() { return static_cast<unsigned>(m_textures.size()); }

  private:
    std::vector<std::unique_ptr<DX12Mesh>> m_meshes;
    std::vector<std::unique_ptr<DX12Shader>> m_shaders; // for each model
    std::vector<std::unique_ptr<DX12Texture>> m_textures; // for each model
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
    aiMatrix4x4 m_transformation;
    // workaround
    bool m_isCubeMap;

  private:
    DX12Model(const DX12Model&) = delete;
    DX12Model& operator=(const DX12Model&) = delete;

  };
}

