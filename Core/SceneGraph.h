#pragma once

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

  private:
    SceneGraph();
    SceneGraph(const SceneGraph&) = delete;
    SceneGraph& operator=(const SceneGraph&) = delete;
  };
}
