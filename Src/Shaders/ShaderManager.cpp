#include "stdafx.h"
#include "ShaderManager.h"

#include "Core/DX12Interface.h"

#include <filesystem>
#include <fstream>

// json
#include <json.hpp>
using json = nlohmann::json;

namespace Core
{
  ShaderManager::ShaderManager()
    : m_shaderMap()
  {
    RegisterShaders();
  }

  ShaderManager::~ShaderManager()
  {
    m_shaderMap.clear();
  }

  void ShaderManager::RegisterShaders()
  {
    // TODO: handle errors
    auto configPath = std::filesystem::current_path().string() + "/Configs/Shaders.json";

    // read config file and parse it
    json configData = json::parse(std::ifstream(configPath))["Shaders"];

    for (auto data : configData)
    {
      std::string name;
      std::string type;
      std::string path;
      
      data.at("Name").get_to(name);
      data.at("Type").get_to(type);
      data.at("Path").get_to(path);

      // create shader blob, which will compile the shader for now
      m_shaderMap[name] = std::make_shared<ShaderBlob>(path, type == "Compute");
    }
  }
}
