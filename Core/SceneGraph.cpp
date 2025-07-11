#include "stdafx.h"
#include "SceneGraph.h"

#include <filesystem>
#include <fstream>

// json
#include <json.hpp>
using json = nlohmann::json;

namespace Core
{
  SceneGraph::SceneGraph()
    : m_models()
  {
    // TODO: handle errors
    auto configPath = std::filesystem::current_path().string() + "/Configs/Scene.json";

    // read config file and parse it
    json configData = json::parse(std::ifstream(configPath));

    std::string name;
    configData.at("Name").get_to(name);

    auto objects = configData["Objects"];

    for (auto object : objects)
    {
      std::string path;
      object.at("Path").get_to(path);
      
      auto model = new DX12Model();
      model->LoadModel(path.c_str());
      m_models.push_back(model);

      // this currently done from application
      // model->SetupModel(m_context->GetCommandList());
    }
  }

  SceneGraph::~SceneGraph()
  {
    // delete models
    for (auto model : m_models)
      delete model;
    m_models.clear();
  }

  const std::vector<DX12Model*>& SceneGraph::GetModels()
  {
    return m_models;
  }
}
