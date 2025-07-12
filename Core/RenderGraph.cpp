#include "stdafx.h"
#include "RenderGraph.h"

#include <filesystem>
#include <fstream>

// json
#include <json.hpp>
using json = nlohmann::json;

namespace Core
{
  RenderGraph::RenderGraph()
    : m_passes()
  {
    // TODO: handle errors
    auto configPath = std::filesystem::current_path().string() + "/Configs/RenderGraph.json";

    // read config file and parse it
    json configData = json::parse(std::ifstream(configPath))["RenderGraph"];
    auto passes = configData["Passes"];

    for (auto pass : passes)
    {
      std::string name;
      std::string pso;

      pass.at("Name").get_to(name);
      pass.at("PSO").get_to(pso);

      // just plain pass for now
      m_passes[name] = new RenderPass();
    }
  }

  RenderGraph::~RenderGraph()
  {
    for (auto& pass : m_passes)
      delete pass.second;

    m_passes.clear();
  }
}
