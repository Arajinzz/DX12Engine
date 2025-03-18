#include "stdafx.h"
#include "DX12Model.h"

#include "Core/DX12Device.h"
#include "Core/DXApplicationHelper.h"
#include "Core/DX12FrameResource.h"
#include "Core/WindowsApplication.h"

#include <fstream>

#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>

namespace Core
{
  DX12Model::DX12Model()
    : m_vertices()
    , m_indices()
    , m_bundle(nullptr)
    , m_descHeap(nullptr)
    , m_constantBuffer(nullptr)
  {
    // create heap
    m_descHeap = std::make_unique<DX12Heap>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    // create constant buffer
    m_constantBuffer = std::make_unique<DX12ConstantBuffer>();

    // Create the command list.
    // the class should add command list automatically to CommandQueue
    m_bundle = std::make_unique<DX12CommandList>(D3D12_COMMAND_LIST_TYPE_BUNDLE);
  }

  DX12Model::~DX12Model()
  {
    m_bundle.reset();
    m_descHeap.reset();
  }

  void DX12Model::Setup()
  {
    FrameResource().GetShader()->AddParameter(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX);

    // order has to be correct. FrameResource already adds CBV then SRV
    // so we have CBV, SRV, CBV
    m_descHeap->AddResource(FrameResource().GetConstantBuffer()->GetResource(), CONSTANTBUFFER);
    m_descHeap->AddResource(FrameResource().GetTexture()->GetResource(), TEXTURE);
    m_descHeap->AddResource(m_constantBuffer->GetResource(), CONSTANTBUFFER);
    
    // constant buffers
    FrameResource().GetConstantBuffer()->MapToResource(m_descHeap->GetResource(0));
    m_constantBuffer->MapToResource(m_descHeap->GetResource(2));

    FrameResource().GetShader()->CreateRootSignature();

    // Define the vertex input layout.
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
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
  }

  void DX12Model::Draw(unsigned frameIndex)
  {
    // 1 allocator
    m_bundle->Reset(frameIndex, m_pipelineState.Get());
    // Set necessary state.
    m_bundle->SetRootSignature(FrameResource().GetShader()->GetRootSignature());
    m_bundle->SetDescriptorHeap(m_descHeap.get());
    
    auto handle = m_descHeap->Get()->GetGPUDescriptorHandleForHeapStart();
    m_bundle->Get()->SetGraphicsRootDescriptorTable(0, handle);

    handle.ptr += Device()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    m_bundle->Get()->SetGraphicsRootDescriptorTable(1, handle);

    handle.ptr += Device()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    m_bundle->Get()->SetGraphicsRootDescriptorTable(2, handle);

    m_bundle->Get()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_bundle->Get()->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    m_bundle->Get()->IASetIndexBuffer(&m_indexBufferView);
    m_bundle->Get()->DrawIndexedInstanced(m_indices.size(), m_indices.size() / 3, 0, 0, 0);

    m_bundle->Close();
  }

  void DX12Model::LoadModel(const char* path)
  {
    Assimp::Importer importer;

    const aiScene* pModel = importer.ReadFile(path,
      aiProcess_Triangulate |
      aiProcess_JoinIdenticalVertices |
      aiProcess_ConvertToLeftHanded);

    const auto pMesh = pModel->mMeshes[0];

    m_vertices.reserve(pMesh->mNumVertices);

    for (unsigned i = 0; i < pMesh->mNumVertices; ++i)
    {
      Vertex vertex;
      vertex.position = { pMesh->mVertices[i].x, pMesh->mVertices[i].y, pMesh->mVertices[i].z };
      float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
      float g = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
      float b = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
      vertex.color = { r, g, b, 1.0};
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

    const unsigned vertexBufferSize = m_vertices.size() * sizeof(Vertex);
    const unsigned indexBufferSize = m_indices.size() * sizeof(uint16_t);

    {
      // Note: using upload heaps to transfer static data like vert buffers is not 
      // recommended. Every time the GPU needs it, the upload heap will be marshalled 
      // over. Please read up on Default Heap usage. An upload heap is used here for 
      // code simplicity and because there are very few verts to actually transfer.
      auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
      auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
      ThrowIfFailed(Device()->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_vertexBuffer)));

      // Copy the triangle data to the vertex buffer.
      UINT8* pVertexDataBegin;
      CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
      ThrowIfFailed(m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
      memcpy(pVertexDataBegin, m_vertices.data(), vertexBufferSize);
      m_vertexBuffer->Unmap(0, nullptr);

      // Initialize the vertex buffer view.
      m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
      m_vertexBufferView.StrideInBytes = sizeof(Vertex);
      m_vertexBufferView.SizeInBytes = vertexBufferSize;
    }

    {
      auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
      auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize);
      ThrowIfFailed(Device()->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_indexBuffer)));

      // Copy the index data to the index buffer.
      UINT8* pIndexDataBegin;
      CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
      ThrowIfFailed(m_indexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pIndexDataBegin)));
      memcpy(pIndexDataBegin, m_indices.data(), indexBufferSize);
      m_indexBuffer->Unmap(0, nullptr);

      // Initialize the index buffer view.
      m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
      m_indexBufferView.SizeInBytes = indexBufferSize;
      m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
    }

    m_bundle->Close();
  }

  void DX12Model::Update()
  {
    auto static angle = 0.0;
    angle += 3 * WindowsApplication::deltaTime;
    if (angle > 360.0)
      angle = 0.0;

    m_constantBuffer->SetModel(XMMatrixTranspose(
      XMMatrixIdentity() * XMMatrixRotationZ(angle) * XMMatrixRotationY(angle * 2) * XMMatrixRotationX(angle) * XMMatrixTranslation(sin(angle), 0.0, 0.0)
    ));
  }
}