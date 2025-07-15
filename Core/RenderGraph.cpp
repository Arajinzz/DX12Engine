#include "stdafx.h"
#include "RenderGraph.h"

#include "Core/BasePass.h"
#include "Core/SkyboxPass.h"
#include "Core/PSOManager.h"
#include "Core/ComposerPass.h"

#include <filesystem>
#include <fstream>

// json
#include <json.hpp>
using json = nlohmann::json;

namespace Core
{
  RenderGraph::RenderGraph()
    : m_passesMap()
    , m_passesVec()
    , m_creators()
  {
    // register creators
    // key has to match name in RenderGraph.json
    m_creators["BasePass"] = [&]() { return new BasePass(); };
    m_creators["SkyboxPass"] = [&]() { return new SkyboxPass(); };
    m_creators["ComposerPass"] = [&]() { return new ComposerPass(); };

    // read the config file
    ReadRenderGraph();
  }

  RenderGraph::~RenderGraph()
  {
    for (auto& pass : m_passesVec)
      delete pass;

    m_passesMap.clear();
    m_passesVec.clear();
    m_creators.clear();
  }

  const std::vector<RenderPass*>& RenderGraph::GetPasses()
  {
    return m_passesVec;
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
      m_passesMap[name] = m_creators[name](); // create
      // set pso and root signature
      m_passesMap[name]->SetPSO(PSOManager::Instance().GetPSO(pso));
      m_passesMap[name]->SetRootSignature(PSOManager::Instance().GetRootSignature(pso));
      // store in vec to maintain order of config file
      m_passesVec.emplace_back(m_passesMap[name]);
    }
  }

}
