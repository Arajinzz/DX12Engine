#include "stdafx.h"
#include "RenderGraph.h"

#include "Core/BasePass.h"
#include "Core/SkyboxPass.h"
#include "Core/PSOManager.h"

#include <filesystem>
#include <fstream>

// json
#include <json.hpp>
using json = nlohmann::json;

namespace Core
{
  RenderGraph::RenderGraph()
    : m_passes()
    , m_creators()
  {
    // register creators
    // key has to match name in RenderGraph.json
    m_creators["BasePass"] = [&]() { return new BasePass(); };
    m_creators["SkyboxPass"] = [&]() { return new SkyboxPass(); };

    // read the config file
    ReadRenderGraph();
  }

  RenderGraph::~RenderGraph()
  {
    for (auto& pass : m_passes)
      delete pass.second;

    m_passes.clear();
    m_creators.clear();
  }

  void RenderGraph::ReadRenderGraph()
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
      m_passes[name] = m_creators[name](); // create
      // set pso
      m_passes[name]->SetPSO(PSOManager::Instance().GetPSO(pso));
    }
  }

}
