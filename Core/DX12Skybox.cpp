#include "stdafx.h"
#include "DX12Skybox.h"

namespace Core
{
  DX12Skybox::DX12Skybox()
    : m_cubeMap(nullptr)
    , m_cube(nullptr)
  {
    std::vector<std::string> paths(6);
    paths[0] = "skybox\\bluecloud_rt.jpg";
    paths[1] = "skybox\\bluecloud_lt.jpg";
    paths[2] = "skybox\\bluecloud_up.jpg";
    paths[3] = "skybox\\bluecloud_dn.jpg";
    paths[4] = "skybox\\bluecloud_ft.jpg";
    paths[5] = "skybox\\bluecloud_bk.jpg";

    m_cube = std::make_unique<DX12Mesh>();
    m_cube->LoadMesh("models\\cube.obj");
    m_cubeMap = std::make_unique<DX12Texture>(paths);
  }

  DX12Skybox::~DX12Skybox()
  {
  }

  void DX12Skybox::Draw()
  {
  }

  void DX12Skybox::Update()
  {
  }
}