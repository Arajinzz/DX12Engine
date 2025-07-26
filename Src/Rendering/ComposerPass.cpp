#include "stdafx.h"
#include "ComposerPass.h"

#include "Core/TextureManager.h"
#include "Core/DX12Interface.h"
#include "Core/DXApplicationHelper.h"

// ARTIFACTS happens because resizing is not working correctly
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
    , m_quadReady(false)
  {
    // vertex data
    m_vertices[0] = Vertex({ -1.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f });
    m_vertices[1] = Vertex({  1.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f });
    m_vertices[2] = Vertex({ -1.0f,  1.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f });
    m_vertices[3] = Vertex({  1.0f,  1.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f });
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
    m_vertexBuffer.Reset();
    m_indexBuffer.Reset();
    m_vertexBufferUploadHeap.Reset();
    m_indexBufferUploadHeap.Reset();
  }

  void ComposerPass::Render(DX12Context* ctx)
  {
    auto commandList = ctx->GetCommandList();
    
    if (!m_quadReady)
    {
      SetupVertexBuffer(commandList);
      SetupIndexBuffer(commandList);
      m_quadReady = true;
    }

    // set render target and depth buffer
    // this pass has to set the render target to swap back buffer
    auto renderTarget = ctx->GetCurrentRenderTarget();
    // no need to to transition this is handled by the context
    // it is exepected at this point the render target should be in render target state
    auto rtvHandle = ResourceManager::Instance().GetRTVCpuHandle(renderTarget->index);
    auto dsvHandle = ResourceManager::Instance().GetDSVCpuHandle(0);
    commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
    const float clearColor[] = { 0.25f, 0.55f, 0.45f, 1.0f };
    commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    commandList->SetPipelineState(m_pso);
    commandList->SetGraphicsRootSignature(m_rootSignature);

    ID3D12DescriptorHeap* ppHeaps[] = { ResourceManager::Instance().GetResourcesHeap() };
    commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

    // texture SRV
    commandList->SetGraphicsRootDescriptorTable(0, ResourceManager::Instance().GetResourceGpuHandle(renderTarget->activeSRVIndex));

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    commandList->IASetIndexBuffer(&m_indexBufferView);
    commandList->DrawIndexedInstanced(6 /* indices */, 1, 0, 0, 0);
  }

  void ComposerPass::SetupVertexBuffer(ID3D12GraphicsCommandList* commandList)
  {
    const unsigned vertexBufferSize = static_cast<unsigned>(sizeof(m_vertices));
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
    vertexData.pData = m_vertices;
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

  void ComposerPass::SetupIndexBuffer(ID3D12GraphicsCommandList* commandList)
  {
    const unsigned indexBufferSize = static_cast<unsigned>(sizeof(m_indices));
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
    indexData.pData = m_indices;
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