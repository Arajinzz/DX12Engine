#pragma once

#include "Core/DX12Model.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace Core
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

    const std::vector<DX12Model*>& GetModels();

  private:
    // meshes
    std::vector<DX12Model*> m_models;

  private:
    SceneGraph();
    SceneGraph(const SceneGraph&) = delete;
    SceneGraph& operator=(const SceneGraph&) = delete;
  };
}
