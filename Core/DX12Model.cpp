#include "stdafx.h"
#include "DX12Model.h"

#include <fstream>

#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>

namespace Core
{
  DX12Model::DX12Model()
    : m_vertices()
    , m_indices()
  {
  }

  DX12Model::~DX12Model()
  {
  }

  void DX12Model::LoadModel(std::string& path)
  {
    Assimp::Importer importer;

    const aiScene* pScene = importer.ReadFile(path,
      aiProcess_Triangulate |
      aiProcess_ConvertToLeftHanded);
  }
}