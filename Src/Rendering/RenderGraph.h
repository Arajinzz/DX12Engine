#pragma once

#include "Core/RenderPass.h"

#include <functional>
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

    // get passes in order from config
    const std::vector<RenderPass*>& GetPasses();

  private:
    void ReadRenderGraph();

  private:
    using Creator = std::function<RenderPass*()>;
    // creator
    std::unordered_map<std::string, Creator> m_creators;
    // passes map
    std::unordered_map<std::string, RenderPass*> m_passesMap;
    // to maintain passes order
    std::vector<RenderPass*> m_passesVec;

  private:
    RenderGraph();
    RenderGraph(const RenderGraph&) = delete;
    RenderGraph& operator=(const RenderGraph&) = delete;
  };

}

