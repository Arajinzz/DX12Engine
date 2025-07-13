#include "stdafx.h"
#include "DX12Skybox.h"

#include "Core/TextureManager.h"

namespace Core
{
  DX12Skybox::DX12Skybox()
    : DX12Model()
  {
  }

  DX12Skybox::~DX12Skybox()
  {
  }

  void DX12Skybox::LoadModel(const char* path)
  {
    Assimp::Importer importer;

    const aiScene* pModel = importer.ReadFile(path,
      aiProcess_Triangulate |
      aiProcess_JoinIdenticalVertices |
      aiProcess_ConvertToLeftHanded |
      aiProcess_GenNormals |
      aiProcess_CalcTangentSpace);

    aiMatrix4x4 identity; // identity matrix
    const auto pMesh = pModel->mMeshes[0];
    auto mesh = std::make_unique<DX12Mesh>();
    mesh->LoadMesh(pMesh, identity);

    std::vector<std::string> paths;
    paths.push_back("skybox\\bluecloud_ft.jpg");
    paths.push_back("skybox\\bluecloud_bk.jpg");
    paths.push_back("skybox\\bluecloud_up.jpg");
    paths.push_back("skybox\\bluecloud_dn.jpg");
    paths.push_back("skybox\\bluecloud_rt.jpg");
    paths.push_back("skybox\\bluecloud_lf.jpg");

    m_meshes.emplace_back(mesh.release());
    m_textures.emplace_back(TextureManager::Instance().CreateOrGetTexture(paths));
  }
}