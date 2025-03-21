#include "stdafx.h"
#include "DX12Model.h"

#include "Core/DX12Device.h"
#include "Core/DXApplicationHelper.h"
#include "Core/DX12FrameResource.h"
#include "Core/WindowsApplication.h"

#include <fstream>

namespace Core
{
  DX12Model::DX12Model()
    : m_vertices()
    , m_indices()
    , m_bundle(nullptr)
  {
    // Create the command list.
    // the class should add command list automatically to CommandQueue
    m_bundle = std::make_unique<DX12CommandList>(D3D12_COMMAND_LIST_TYPE_BUNDLE);
    m_bundle->Close();
  }

  DX12Model::~DX12Model()
  {
    m_bundle.reset();
  }

  void DX12Model::Setup(ID3D12GraphicsCommandList* commandList)
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
    psoDesc.pRootSignature = FrameResource().GetShader()->GetRootSignature();
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(FrameResource().GetShader()->GetVertexShader());
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(FrameResource().GetShader()->GetPixelShader());
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable = TRUE;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    psoDesc.SampleDesc.Count = 1;
    ThrowIfFailed(Device()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));

    SetupVertexBuffer(commandList);
    SetupIndexBuffer(commandList);
  }

  void DX12Model::Draw(unsigned frameIndex, DX12Heap* heapDesc)
  {
    // 1 allocator
    m_bundle->Reset(frameIndex, m_pipelineState.Get());
    // Set necessary state.
    m_bundle->SetRootSignature(FrameResource().GetShader()->GetRootSignature());
    m_bundle->SetDescriptorHeap(heapDesc);
    
    auto handle = heapDesc->Get()->GetGPUDescriptorHandleForHeapStart();
    m_bundle->Get()->SetGraphicsRootDescriptorTable(0, handle);

    handle.ptr += Device()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    m_bundle->Get()->SetGraphicsRootDescriptorTable(1, handle);

    handle.ptr += Device()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    m_bundle->Get()->SetGraphicsRootDescriptorTable(2, handle);

    m_bundle->Get()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_bundle->Get()->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    m_bundle->Get()->IASetIndexBuffer(&m_indexBufferView);
    m_bundle->Get()->DrawIndexedInstanced(m_indices.size(), 1, 0, 0, 0);

    m_bundle->Close();
  }

  void DX12Model::LoadModel(const aiMesh* pMesh)
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

  void DX12Model::SetupVertexBuffer(ID3D12GraphicsCommandList* commandList)
  {
    const unsigned vertexBufferSize = m_vertices.size() * sizeof(Vertex);
    auto uploadHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto defaultHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
    ThrowIfFailed(Device()->CreateCommittedResource(
      &defaultHeapProps,
      D3D12_HEAP_FLAG_NONE,
      &resDesc,
      D3D12_RESOURCE_STATE_COMMON,
      nullptr,
      IID_PPV_ARGS(&m_vertexBuffer)));

    ThrowIfFailed(Device()->CreateCommittedResource(
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
    UpdateSubresources<1>(commandList, m_vertexBuffer.Get(), m_vertexBufferUploadHeap.Get(), 0, 0, 1, &vertexData);
    commandList->ResourceBarrier(1, &barrier2);

    // Initialize the vertex buffer view.
    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.StrideInBytes = sizeof(Vertex);
    m_vertexBufferView.SizeInBytes = vertexBufferSize;
  }

  void DX12Model::SetupIndexBuffer(ID3D12GraphicsCommandList* commandList)
  {
    const unsigned indexBufferSize = m_indices.size() * sizeof(uint16_t);
    auto uploadHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto defaultHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize);
    ThrowIfFailed(Device()->CreateCommittedResource(
      &defaultHeapProps,
      D3D12_HEAP_FLAG_NONE,
      &resDesc,
      D3D12_RESOURCE_STATE_COMMON,
      nullptr,
      IID_PPV_ARGS(&m_indexBuffer)));

    ThrowIfFailed(Device()->CreateCommittedResource(
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
    UpdateSubresources<1>(commandList, m_indexBuffer.Get(), m_indexBufferUploadHeap.Get(), 0, 0, 1, &indexData);
    commandList->ResourceBarrier(1, &barrier2);

    // Initialize the index buffer view.
    m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
    m_indexBufferView.SizeInBytes = indexBufferSize;
    m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
  }
}