#include "stdafx.h"
#include "DX12Skybox.h"

namespace Core
{
  DX12Skybox::DX12Skybox()
    : m_cube(nullptr)
  {
    m_cube = std::make_unique<DX12Mesh>();
    m_cube->LoadMeshSkyboxSpecific("models\\cube.obj");
  }

  DX12Skybox::~DX12Skybox()
  {
  }

  void DX12Skybox::Setup(ID3D12GraphicsCommandList* commandList)
  {
    m_cube->SetupMesh(commandList);
  }

  void DX12Skybox::Update()
  {
    m_cube->UpdateMesh();
  }
}