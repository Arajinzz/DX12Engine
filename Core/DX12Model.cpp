#include "stdafx.h"
#include "DX12Model.h"

#include "Core/DX12FrameResource.h"
#include "Core/WindowsApplication.h"
#include "Core/DX12Texture.h"
#include "Core/DX12Interface.h"
#include "Core/DX12Shader.h"
#include "Core/DX12Mesh.h"

#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>

namespace Core
{
  DX12Model::DX12Model()
    : m_meshes()
    , m_descHeap(nullptr)
    , m_pCbvDataBegin(nullptr)
    , m_constantBufferData()
    , m_constantBuffer(nullptr)
    , m_translation(0.0f, 0.0f, 0.0f)
    , m_scale(1.0f, 1.0f, 1.0f)
    , m_angle(0.0f)
    , m_shaders()
    , m_textures()
    , m_isCubeMap(false)
  {
    // create heap
    m_descHeap = std::make_unique<DX12Heap>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    // create constant buffer
    m_constantBuffer = DX12Interface::Get().CreateConstantBuffer(sizeof(ConstantBufferData), D3D12_HEAP_TYPE_UPLOAD);
    // Map and initialize the constant buffer. We don't unmap this until the
    // app closes. Keeping things mapped for the lifetime of the resource is okay.
    CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
    ThrowIfFailed(m_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_pCbvDataBegin)));
    memcpy(m_pCbvDataBegin, &m_constantBufferData, sizeof(m_constantBufferData));
  }

  DX12Model::~DX12Model()
  {
    m_descHeap.reset();
    m_constantBuffer.Reset();
  }

  void DX12Model::SetupModel(ID3D12GraphicsCommandList* commandList)
  {
    // always Globals first
    m_descHeap->AddResource(FrameResource().GetConstantBuffer(), CONSTANTBUFFER);
    m_descHeap->AddResource(m_constantBuffer, CONSTANTBUFFER);

    for (auto& texture : m_textures)
      m_descHeap->AddResource(texture->GetResource(), m_isCubeMap ? CUBEMAP : TEXTURE);

    for (const auto& texture : m_textures)
      texture->CopyToGPU(commandList);

    for (int i = 0; i < m_meshes.size(); ++i)
    {
      m_meshes[i]->Setup(commandList, m_shaders[i].get());
    }
  }

  void DX12Model::DrawModel(unsigned frameIndex, ID3D12GraphicsCommandList* commandList)
  {
    for (int i = 0; i < m_meshes.size(); ++i)
      m_meshes[i]->Draw(frameIndex, m_descHeap.get(), m_shaders[i].get(), i, commandList);
  }

  void DX12Model::LoadModel(const char* path)
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
      auto mesh = std::make_unique<DX12Mesh>();
      mesh->LoadMesh(pMesh);

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

      m_meshes.emplace_back(mesh.release());
    }
  }

  void DX12Model::LoadModelSkyboxSpecific(const char* path)
  {
    m_isCubeMap = true;

    Assimp::Importer importer;

    const aiScene* pModel = importer.ReadFile(path,
      aiProcess_Triangulate |
      aiProcess_JoinIdenticalVertices |
      aiProcess_ConvertToLeftHanded |
      aiProcess_GenNormals |
      aiProcess_CalcTangentSpace);

    const auto pMesh = pModel->mMeshes[0];
    auto mesh = std::make_unique<DX12Mesh>(D3D12_CULL_MODE_FRONT, false);
    mesh->LoadMesh(pMesh);

    auto shader = std::make_unique<DX12Shader>(L"skybox_shaders.hlsl");
    // add slots
    shader->AddParameter(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX);
    shader->AddParameter(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX);
    shader->AddParameter(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, D3D12_SHADER_VISIBILITY_PIXEL);
    // create rootSignature
    shader->CreateRootSignature();

    std::vector<std::string> paths;
    paths.push_back("skybox\\bluecloud_ft.jpg");
    paths.push_back("skybox\\bluecloud_bk.jpg");
    paths.push_back("skybox\\bluecloud_up.jpg");
    paths.push_back("skybox\\bluecloud_dn.jpg");
    paths.push_back("skybox\\bluecloud_rt.jpg");
    paths.push_back("skybox\\bluecloud_lf.jpg");

    auto cubeMap = std::make_unique<DX12Texture>(paths, 1);

    m_shaders.emplace_back(shader.release());
    m_meshes.emplace_back(mesh.release());
    m_textures.emplace_back(cubeMap.release());
  }

  void DX12Model::UpdateModel()
  {
    /*m_angle += 100 * WindowsApplication::deltaTime;
    if (m_angle > 360.0)
      m_angle = 0.0;*/

      // quick hack to test performance
    if (staticMesh && !isModelSet)
    {
      XMMATRIX T = XMMatrixTranslation(m_translation.x, m_translation.y, m_translation.z);
      XMMATRIX R = XMMatrixRotationY(XMConvertToRadians(m_angle));
      XMMATRIX S = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);

      m_constantBufferData.model = XMMatrixTranspose(S * R * T);
      memcpy(m_pCbvDataBegin, &m_constantBufferData, sizeof(m_constantBufferData));
    }
  }

  unsigned DX12Model::GetTriangleCount()
  {
    unsigned count = 0;
    for (const auto& mesh : m_meshes)
      count += mesh->GetTriangleCount();
    return count;
  }
}