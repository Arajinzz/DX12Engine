#pragma once

#include "Scene/DX12Model.h"
#include "Scene/DX12Camera.h"

#include "Scene/DX12Skybox.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace Scene
{
  class SceneGraph
  {
  public:
    static SceneGraph& Instance()
    {
      static SceneGraph instance;
      return instance;
    }
    ~SceneGraph();

    const Graphics::ResourceDescriptor* GetSceneBuffer() { return m_constantBuffer.get(); }
    DX12Camera* GetCamera() { return m_camera.get(); }
    DX12Skybox* GetSkybox() { return m_skybox.get(); }


    // temporary
    void UpdateScene();
    const std::vector<DX12Model*>& GetModels();

  private:
    // meshes
    std::vector<DX12Model*> m_models;

  private:
    // scene globals
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
    // CB resource
    std::unique_ptr<Graphics::ResourceDescriptor> m_constantBuffer;
    // camera and skybox
    std::unique_ptr<DX12Camera> m_camera;
    std::unique_ptr<DX12Skybox> m_skybox;

  private:
    SceneGraph();
    SceneGraph(const SceneGraph&) = delete;
    SceneGraph& operator=(const SceneGraph&) = delete;
  };
}
