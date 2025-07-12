#pragma once

#include "Core/RenderPass.h"

#include <unordered_map>

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
    // passes in order
    std::unordered_map<std::string, RenderPass*> m_passes;

  private:
    RenderGraph();
    RenderGraph(const RenderGraph&) = delete;
    RenderGraph& operator=(const RenderGraph&) = delete;
  };

}

