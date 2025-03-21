#include "stdafx.h"
#include "DX12Mesh.h"

#include "Core/DX12FrameResource.h"
#include "Core/WindowsApplication.h"
#include "Core/DX12Texture.h"
#include "Core/DX12ConstantBuffer.h"

#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>

namespace Core
{
  DX12Mesh::DX12Mesh()
    : m_models()
    , m_descHeap(nullptr)
    , m_constantBuffer(nullptr)
    , m_translation(0.0f, 0.0f, 0.0f)
    , m_angle(0.0f)
  {
    // create heap
    m_descHeap = std::make_unique<DX12Heap>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    // create constant buffer
    m_constantBuffer = std::make_unique<DX12ConstantBuffer>();
  }

  DX12Mesh::~DX12Mesh()
  {
    m_descHeap.reset();
    m_constantBuffer.reset();
  }

  void DX12Mesh::SetupMesh(ID3D12GraphicsCommandList* commandList)
  {
    std::vector<DX12ConstantBuffer*> constantBuffers = { m_constantBuffer.get() };
    std::vector<DX12Texture*> textures = {};
    FrameResource().InitHeapDesc(m_descHeap.get(), constantBuffers, textures);

    for (const auto& model : m_models)
      model->Setup(commandList);
  }

  void DX12Mesh::DrawMesh(unsigned frameIndex, ID3D12GraphicsCommandList* commandList)
  {
    for (const auto& model : m_models)
    {
      model->Draw(frameIndex, m_descHeap.get());
      commandList->ExecuteBundle(model->GetBundle());
    }
  }

  void DX12Mesh::LoadMesh(const char* path)
  {
    Assimp::Importer importer;

    const aiScene* pModel = importer.ReadFile(path,
      aiProcess_Triangulate |
      aiProcess_JoinIdenticalVertices |
      aiProcess_ConvertToLeftHanded |
      aiProcess_GenNormals |
      aiProcess_CalcTangentSpace);

    m_transformation = pModel->mRootNode->mTransformation;

    for (unsigned i = 0; i < pModel->mNumMeshes; ++i)
    {
      const auto pMesh = pModel->mMeshes[i];
      auto model = std::make_unique<DX12Model>();
      model->LoadModel(pMesh);
      m_models.emplace_back(model.release());

      // testing
      const auto material = pModel->mMaterials[pMesh->mMaterialIndex];
      auto name = material->GetName();
    }
  }

  void DX12Mesh::UpdateMesh()
  {
    /*m_angle += 100 * WindowsApplication::deltaTime;
    if (m_angle > 360.0)
      m_angle = 0.0;*/

    XMMATRIX T = XMMatrixTranslation(m_translation.x, m_translation.y, m_translation.z);
    XMMATRIX R = XMMatrixRotationY(XMConvertToRadians(m_angle));
    XMMATRIX S = XMMatrixScaling(1, 1, 1);

    m_constantBuffer->SetModel(XMMatrixTranspose(S * R * T));
  }

  unsigned DX12Mesh::GetTriangleCount()
  {
    unsigned count = 0;
    for (const auto& model : m_models)
      count += model->GetTriangleCount();
    return count;
  }
}