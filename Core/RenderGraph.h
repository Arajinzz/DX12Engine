#pragma once

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace Core
{
  class RenderGraph
  {
  public:
    static RenderGraph& Instance()
    {
      static RenderGraph instance;
      return instance;
    }
    ~RenderGraph();

  private:
    RenderGraph();
    RenderGraph(const RenderGraph&) = delete;
    RenderGraph& operator=(const RenderGraph&) = delete;
  };

}

