#pragma once

#include "Core/DX12CommandList.h"
#include "Core/DX12Heap.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace Core
{
  class Cube
  {
  public:
    Cube();
    ~Cube();

    void Draw(CD3DX12_CPU_DESCRIPTOR_HANDLE renderTargetHandle);

  private:
    struct Vertex
    {
      XMFLOAT3 position;
      XMFLOAT4 color;
    };

    std::unique_ptr<DX12CommandList> m_commandList;

    // root signature and pso
    ComPtr<ID3D12RootSignature> m_rootSignature;
    ComPtr<ID3D12PipelineState> m_pipelineState;

    ComPtr<ID3D12Resource> m_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
    ComPtr<ID3D12Resource> m_indexBuffer;
    D3D12_INDEX_BUFFER_VIEW m_indexBufferView;

  private:
    Cube(const Cube&) = delete;
    Cube& operator=(const Cube&) = delete;

  };
}

