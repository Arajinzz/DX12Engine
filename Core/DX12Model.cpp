#include "stdafx.h"
#include "DX12Model.h"

#include "Core/DX12FrameResource.h"
#include "Core/WindowsApplication.h"
#include "Core/DX12Texture.h"
#include "Core/DX12Interface.h"
#include "Core/ShaderManager.h"
#include "Core/TextureManager.h"

#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>

namespace Core
{
  DX12Mesh::DX12Mesh(D3D12_CULL_MODE cullMode, bool depthEnabled)
    : m_vertices()
    , m_indices()
  {
  }

  DX12Mesh::~DX12Mesh()
  {
  }

  void DX12Mesh::Setup(ID3D12GraphicsCommandList* commandList)
  {
    SetupVertexBuffer(commandList);
    SetupIndexBuffer(commandList);
  }

  void DX12Mesh::Draw(
    unsigned frameIndex, ResourceDescriptor* cb, TextureDescriptor* texture, bool cubeMap, ID3D12GraphicsCommandList* commandList)
  {
    if (cubeMap)
    {
      commandList->SetPipelineState(ShaderManager::Instance().GetShader(L"skybox_shaders.hlsl").m_pipelineState.Get());
      commandList->SetGraphicsRootSignature(ShaderManager::Instance().GetShader(L"skybox_shaders.hlsl").m_rootSignature.Get());
    } else
    {
      commandList->SetPipelineState(ShaderManager::Instance().GetShader(L"shaders.hlsl").m_pipelineState.Get());
      commandList->SetGraphicsRootSignature(ShaderManager::Instance().GetShader(L"shaders.hlsl").m_rootSignature.Get());
    }
    
    ID3D12DescriptorHeap* ppHeaps[] = { ResourceManager::Instance().GetResourcesHeap() };
    commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

    // scene CBV
    commandList->SetGraphicsRootDescriptorTable(
      0, ResourceManager::Instance().GetResourceGpuHandle(FrameResource().GetConstantBuffer()->index));

    // object CBV
    commandList->SetGraphicsRootDescriptorTable(1, ResourceManager::Instance().GetResourceGpuHandle(cb->index));

    // texture CBV
    commandList->SetGraphicsRootDescriptorTable(2, ResourceManager::Instance().GetResourceGpuHandle(texture->index));

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    commandList->IASetIndexBuffer(&m_indexBufferView);
    commandList->DrawIndexedInstanced(static_cast<unsigned>(m_indices.size()), 1, 0, 0, 0);
  }

  void DX12Mesh::LoadMesh(const aiMesh* pMesh)
  {
    m_vertices.reserve(pMesh->mNumVertices);

    for (unsigned i = 0; i < pMesh->mNumVertices; ++i)
    {
      Vertex vertex;
      vertex.position = { pMesh->mVertices[i].x, pMesh->mVertices[i].y, pMesh->mVertices[i].z };
      if (pMesh->HasNormals())
        vertex.normal = { pMesh->mNormals[i].x, pMesh->mNormals[i].y, pMesh->mNormals[i].z };
      if (pMesh->HasTextureCoords(0))
        vertex.uv = { pMesh->mTextureCoords[0][i].x, pMesh->mTextureCoords[0][i].y };
      m_vertices.push_back(vertex);
    }

    m_indices.reserve(pMesh->mNumFaces * 3);

    for (unsigned i = 0; i < pMesh->mNumFaces; ++i)
    {
      const auto& face = pMesh->mFaces[i];
      assert(face.mNumIndices == 3);
      m_indices.push_back(face.mIndices[0]);
      m_indices.push_back(face.mIndices[1]);
      m_indices.push_back(face.mIndices[2]);
    }
  }

  void DX12Mesh::SetupVertexBuffer(ID3D12GraphicsCommandList* commandList)
  {
    const unsigned vertexBufferSize = static_cast<unsigned>(m_vertices.size() * sizeof(Vertex));
    auto uploadHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto defaultHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
    ThrowIfFailed(DX12Interface::Get().GetDevice()->CreateCommittedResource(
      &defaultHeapProps,
      D3D12_HEAP_FLAG_NONE,
      &resDesc,
      D3D12_RESOURCE_STATE_COMMON,
      nullptr,
      IID_PPV_ARGS(&m_vertexBuffer)));

    ThrowIfFailed(DX12Interface::Get().GetDevice()->CreateCommittedResource(
      &uploadHeapProps,
      D3D12_HEAP_FLAG_NONE,
      &resDesc,
      D3D12_RESOURCE_STATE_GENERIC_READ,
      nullptr,
      IID_PPV_ARGS(&m_vertexBufferUploadHeap)));

    // copy data to intermediate upload heap
    D3D12_SUBRESOURCE_DATA vertexData = {};
    vertexData.pData = m_vertices.data();
    vertexData.RowPitch = vertexBufferSize;
    vertexData.SlicePitch = vertexData.RowPitch;

    auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
    auto barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    commandList->ResourceBarrier(1, &barrier1);
    UpdateSubresources(commandList, m_vertexBuffer.Get(), m_vertexBufferUploadHeap.Get(), 0, 0, 1, &vertexData);
    commandList->ResourceBarrier(1, &barrier2);

    // Initialize the vertex buffer view.
    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.StrideInBytes = sizeof(Vertex);
    m_vertexBufferView.SizeInBytes = vertexBufferSize;
  }

  void DX12Mesh::SetupIndexBuffer(ID3D12GraphicsCommandList* commandList)
  {
    const unsigned indexBufferSize = static_cast<unsigned>(m_indices.size() * sizeof(uint16_t));
    auto uploadHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto defaultHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize);
    ThrowIfFailed(DX12Interface::Get().GetDevice()->CreateCommittedResource(
      &defaultHeapProps,
      D3D12_HEAP_FLAG_NONE,
      &resDesc,
      D3D12_RESOURCE_STATE_COMMON,
      nullptr,
      IID_PPV_ARGS(&m_indexBuffer)));

    ThrowIfFailed(DX12Interface::Get().GetDevice()->CreateCommittedResource(
      &uploadHeapProps,
      D3D12_HEAP_FLAG_NONE,
      &resDesc,
      D3D12_RESOURCE_STATE_GENERIC_READ,
      nullptr,
      IID_PPV_ARGS(&m_indexBufferUploadHeap)));

    // Copy data to the intermediate upload heap and then schedule a copy 
        // from the upload heap to the index buffer.
    D3D12_SUBRESOURCE_DATA indexData = {};
    indexData.pData = m_indices.data();
    indexData.RowPitch = indexBufferSize;
    indexData.SlicePitch = indexData.RowPitch;

    auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(m_indexBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
    auto barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(m_indexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
    commandList->ResourceBarrier(1, &barrier1);
    UpdateSubresources(commandList, m_indexBuffer.Get(), m_indexBufferUploadHeap.Get(), 0, 0, 1, &indexData);
    commandList->ResourceBarrier(1, &barrier2);

    // Initialize the index buffer view.
    m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
    m_indexBufferView.SizeInBytes = indexBufferSize;
    m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
  }
}

namespace Core
{
  DX12Model::DX12Model()
    : m_meshes()
    , m_pCbvDataBegin(nullptr)
    , m_constantBufferData()
    , m_constantBuffer(nullptr)
    , m_translation(0.0f, 0.0f, 0.0f)
    , m_scale(1.0f, 1.0f, 1.0f)
    , m_angle(0.0f)
    , m_textures()
    , m_isCubeMap(false)
  {
    // create constant buffer
    m_constantBuffer = ResourceManager::Instance().CreateConstantBufferResource(
      sizeof(ConstantBufferData), D3D12_HEAP_TYPE_UPLOAD);
    // Map and initialize the constant buffer. We don't unmap this until the
    // app closes. Keeping things mapped for the lifetime of the resource is okay.
    CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
    ThrowIfFailed(m_constantBuffer->resource->Map(0, &readRange, reinterpret_cast<void**>(&m_pCbvDataBegin)));
    memcpy(m_pCbvDataBegin, &m_constantBufferData, sizeof(m_constantBufferData));
  }

  DX12Model::~DX12Model()
  {
    m_textures.clear();
  }

  void DX12Model::SetupModel(ID3D12GraphicsCommandList* commandList)
  {
    for (auto& texture : m_textures)
    {
      texture->CopyToGPU(commandList);
      texture->GenerateMips(commandList);
    }

    for (int i = 0; i < m_meshes.size(); ++i)
      m_meshes[i]->Setup(commandList);
  }

  void DX12Model::DrawModel(unsigned frameIndex, ID3D12GraphicsCommandList* commandList)
  {
    for (int i = 0; i < m_meshes.size(); ++i)
      m_meshes[i]->Draw(frameIndex, m_constantBuffer.get(), m_textures[i]->GetResource(), m_isCubeMap, commandList);
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

      const auto material = pModel->mMaterials[pMesh->mMaterialIndex];
      aiString texturePath;
      std::vector<std::string> paths(1);
      // else default texture
      paths[0] = material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS ? texturePath.C_Str() : "textures\\brick.png";

      m_textures.emplace_back(TextureManager::Instance().CreateOrGetTexture(paths));
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