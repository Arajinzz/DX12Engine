#include "stdafx.h"
#include "ComposerPass.h"

#include "Core/TextureManager.h"

namespace Core
{
  ComposerPass::ComposerPass()
    : RenderPass()
    , m_vertices()
    , m_indices()
    , m_vertexBuffer()
    , m_vertexBufferUploadHeap()
    , m_vertexBufferView()
    , m_indexBuffer()
    , m_indexBufferUploadHeap()
    , m_indexBufferView()
  {
    // vertex data
    m_vertices[0] = Vertex({ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f });
    m_vertices[1] = Vertex({ 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f });
    m_vertices[2] = Vertex({ 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f });
    m_vertices[3] = Vertex({ 1.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f });
    // index data
    m_indices[0] = 0;
    m_indices[1] = 1;
    m_indices[2] = 2;
    m_indices[3] = 1;
    m_indices[4] = 3;
    m_indices[5] = 2;
  }

  ComposerPass::~ComposerPass()
  {
  }

  void ComposerPass::Render(DX12Context* ctx)
  {
    return; // not active yet

    auto commandList = ctx->GetCommandList();
    std::vector<std::string> paths(1); 
    // test
    paths[0] = "textures\\brick.png";
    auto texture = TextureManager::Instance().CreateOrGetTexture(paths);

    commandList->SetPipelineState(m_pso);
    commandList->SetGraphicsRootSignature(m_rootSignature);

    ID3D12DescriptorHeap* ppHeaps[] = { ResourceManager::Instance().GetResourcesHeap() };
    commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

    // texture SRV
    commandList->SetGraphicsRootDescriptorTable(0, ResourceManager::Instance().GetResourceGpuHandle(texture->GetResource()->index));

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    commandList->IASetIndexBuffer(&m_indexBufferView);
    commandList->DrawIndexedInstanced(6 /* indices */, 1, 0, 0, 0);
  }
}