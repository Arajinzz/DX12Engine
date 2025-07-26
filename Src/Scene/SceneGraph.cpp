#include "stdafx.h"
#include "SceneGraph.h"

#include "Utilities/DXApplicationHelper.h"

#include <filesystem>
#include <fstream>

// json
#include <json.hpp>
using json = nlohmann::json;

namespace Scene
{
  SceneGraph::SceneGraph()
    : m_models()
    , m_constantBuffer(nullptr)
    , m_camera(nullptr)
    , m_pCbvDataBegin(nullptr)
    , m_constantBufferData()
    , m_skybox(nullptr)
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

    // create camera
    m_camera = std::make_unique<DX12Camera>(45.0f, 0.5f, 10000.0f);
    // create the buffer
    m_constantBuffer = ResourceManager::Instance().CreateConstantBufferResource(
      sizeof(ConstantBufferData), D3D12_HEAP_TYPE_UPLOAD);
    // Map and initialize the constant buffer. We don't unmap this until the
    // app closes. Keeping things mapped for the lifetime of the resource is okay.
    CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
    ThrowIfFailed(m_constantBuffer->resource.Get()->Map(0, &readRange, reinterpret_cast<void**>(&m_pCbvDataBegin)));
    memcpy(m_pCbvDataBegin, &m_constantBufferData, sizeof(m_constantBufferData));

    m_skybox = std::make_unique<DX12Skybox>();
    m_skybox->SetScale(XMFLOAT3(1.0f, 1.0f, 1.0f));
  }

  SceneGraph::~SceneGraph()
  {
    // delete models
    for (auto model : m_models)
      delete model;
    m_models.clear();

    m_constantBuffer.reset();
    m_camera.reset();
    m_skybox.reset();
  }

  void SceneGraph::UpdateScene()
  {
    m_camera->Update();
    m_skybox->UpdateModel();

    m_constantBufferData.view = m_camera->GetView();
    m_constantBufferData.projection = m_camera->GetProjection();
    memcpy(m_pCbvDataBegin, &m_constantBufferData, sizeof(m_constantBufferData));
  }

  const std::vector<DX12Model*>& SceneGraph::GetModels()
  {
    return m_models;
  }
}
