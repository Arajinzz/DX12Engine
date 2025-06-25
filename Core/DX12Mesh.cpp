#include "stdafx.h"
#include "DX12Mesh.h"

#include "Core/DX12Interface.h"
#include "Core/DXApplicationHelper.h"
#include "Core/DX12FrameResource.h"
#include "Core/WindowsApplication.h"
#include "Core/DX12Texture.h"
#include "Core/DX12Shader.h"

#include <fstream>

namespace Core
{
  DX12Mesh::DX12Mesh(D3D12_CULL_MODE cullMode, bool depthEnabled)
    : m_vertices()
    , m_indices()
    , m_cullMode(cullMode)
    , m_depthEnabled(depthEnabled)
  {
  }

  DX12Mesh::~DX12Mesh()
  {
  }

  void DX12Mesh::Setup(ID3D12GraphicsCommandList* commandList, DX12Shader* shader)
  {
    // Define the vertex input layout.
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    // Describe and create the graphics pipeline state object (PSO).
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
    psoDesc.pRootSignature = shader->GetRootSignature();
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(shader->GetVertexShader());
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(shader->GetPixelShader());
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.RasterizerState.CullMode = m_cullMode;
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable = m_depthEnabled;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    psoDesc.SampleDesc.Count = 1;
    ThrowIfFailed(DX12Interface::Get().GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));

    SetupVertexBuffer(commandList);
    SetupIndexBuffer(commandList);
  }

  void DX12Mesh::Draw(
    unsigned frameIndex, ResourceDescriptor* cb, TextureDescriptor* texture, DX12Shader* shader, ID3D12GraphicsCommandList* commandList)
  {
    // 1 allocator
    commandList->SetPipelineState(m_pipelineState.Get());
    // Set necessary state.
    commandList->SetGraphicsRootSignature(shader->GetRootSignature());
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