#include "stdafx.h"
#include "Cube.h"

#include "Core/DX12Device.h"
#include "Core/DXApplicationHelper.h"

namespace Core
{
  Cube::Cube(ID3D12GraphicsCommandList* cmdList)
    : m_commandList(cmdList)
  {
    {
      Vertex triangleVertices[] =
      {
        { { -0.5f, -0.5f, 0.0f }, {1.0, 0.0, 0.0, 1.0} },
        { { 0.5f, -0.5f, 0.0f }, {0.0, 1.0, 0.0, 1.0} },
        { { 0.0f, 0.5f, 0.0f }, {0.0, 0.0, 1.0, 1.0} }
      };
      // since array is on the stack it can deduce the size
      const unsigned vertexBufferSize = sizeof(triangleVertices);

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
      memcpy(pVertexDataBegin, triangleVertices, vertexBufferSize);
      m_vertexBuffer->Unmap(0, nullptr);

      // Initialize the vertex buffer view.
      m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
      m_vertexBufferView.StrideInBytes = sizeof(Vertex);
      m_vertexBufferView.SizeInBytes = vertexBufferSize;
    }

    {
      uint16_t indicies[] = {
        2, 1, 0
      };

      const unsigned indexBufferSize = sizeof(indicies);

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
      memcpy(pIndexDataBegin, indicies, indexBufferSize);
      m_indexBuffer->Unmap(0, nullptr);

      // Initialize the index buffer view.
      m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
      m_indexBufferView.SizeInBytes = indexBufferSize;
      m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
    }
  }

  Cube::~Cube()
  {
  }

  // just a triangle for now
  void Cube::Draw()
  {
    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    m_commandList->IASetIndexBuffer(&m_indexBufferView);
    m_commandList->DrawIndexedInstanced(3, UINT(3 / 3), 0, 0, 0);
  }
}