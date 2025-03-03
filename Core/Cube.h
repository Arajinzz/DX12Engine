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
    Cube(unsigned viewportWidth, unsigned viewportHeight);
    ~Cube();

    void Draw(DX12Heap* rtvHeap, unsigned frameIndex);
    ID3D12PipelineState* GetPSO() { return m_pipelineState.Get(); }

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

    CD3DX12_VIEWPORT m_viewport;
    CD3DX12_RECT m_scissorRect;

  private:
    Cube(const Cube&) = delete;
    Cube& operator=(const Cube&) = delete;

  };
}

