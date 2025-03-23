#include "stdafx.h"
#include "DX12Mesh.h"

#include "Core/DX12FrameResource.h"
#include "Core/WindowsApplication.h"
#include "Core/DX12Texture.h"
#include "Core/DX12ConstantBuffer.h"
#include "Core/DX12Shader.h"

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
    , m_shaders()
    , m_textures()
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
    
    // always Globals first
    m_descHeap->AddResource(FrameResource().GetConstantBuffer()->GetResource(), CONSTANTBUFFER);
    for (auto constantBuffer : constantBuffers)
      m_descHeap->AddResource(constantBuffer->GetResource(), CONSTANTBUFFER);

    for (auto& texture : m_textures)
      m_descHeap->AddResource(texture->GetResource(), TEXTURE);

    for (const auto& texture : m_textures)
      texture->CopyToGPU(commandList);

    for (int i = 0 ; i < m_models.size(); ++i)
    {
      m_models[i]->Setup(commandList, m_shaders[i].get());
    }
  }

  void DX12Mesh::DrawMesh(unsigned frameIndex, ID3D12GraphicsCommandList* commandList)
  {
    for (int i = 0; i < m_models.size(); ++i)
    {
      m_models[i]->Draw(frameIndex, m_descHeap.get(), m_shaders[i].get(), i);
      commandList->ExecuteBundle(m_models[i]->GetBundle());
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

      auto shader = std::make_unique<DX12Shader>(L"shaders.hlsl"); // for each model
      // add slots
      shader->AddParameter(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX);
      shader->AddParameter(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX);
      shader->AddParameter(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, D3D12_SHADER_VISIBILITY_PIXEL);
      
      // create rootsignature
      shader->CreateRootSignature();

      m_shaders.emplace_back(shader.release());

      const auto material = pModel->mMaterials[pMesh->mMaterialIndex];
      aiString texturePath;
      std::vector<std::string> paths(1);
      if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS)
      {
        paths[0] = texturePath.C_Str();
        auto texture = std::make_unique<DX12Texture>(paths);
        m_textures.emplace_back(texture.release());
      } else
      {
        // default texture
        paths[0] = "textures\\brick.png";
        auto texture = std::make_unique<DX12Texture>(paths);
        m_textures.emplace_back(texture.release());
      }

      m_models.emplace_back(model.release());
    }
  }

  void DX12Mesh::LoadMeshSkyboxSpecific(const char* path)
  {
    Assimp::Importer importer;

    const aiScene* pModel = importer.ReadFile(path,
      aiProcess_Triangulate |
      aiProcess_JoinIdenticalVertices |
      aiProcess_ConvertToLeftHanded |
      aiProcess_GenNormals |
      aiProcess_CalcTangentSpace);

    const auto pMesh = pModel->mMeshes[0];
    auto model = std::make_unique<DX12Model>();
    model->LoadModel(pMesh);

    auto shader = std::make_unique<DX12Shader>(L"skybox_shaders.hlsl");
    // add slots
    shader->AddParameter(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX);
    shader->AddParameter(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX);
    shader->AddParameter(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, D3D12_SHADER_VISIBILITY_PIXEL);
    // create rootSignature
    shader->CreateRootSignature();

    std::vector<std::string> paths(6);
    paths[0] = "skybox\\bluecloud_rt.jpg";
    paths[1] = "skybox\\bluecloud_lf.jpg";
    paths[2] = "skybox\\bluecloud_up.jpg";
    paths[3] = "skybox\\bluecloud_dn.jpg";
    paths[4] = "skybox\\bluecloud_ft.jpg";
    paths[5] = "skybox\\bluecloud_bk.jpg";

    auto cubeMap = std::make_unique<DX12Texture>(paths);

    m_shaders.emplace_back(shader.release());
    m_models.emplace_back(model.release());
    m_textures.emplace_back(cubeMap.release());
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