#pragma once

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace Core
{
  class Cube
  {
  public:
    Cube(ID3D12GraphicsCommandList* cmdList);
    ~Cube();

    void Draw();

  private:
    struct Vertex
    {
      XMFLOAT3 position;
      XMFLOAT4 color;
    };

    ID3D12GraphicsCommandList* m_commandList;
    ComPtr<ID3D12Resource> m_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
    ComPtr<ID3D12Resource> m_indexBuffer;
    D3D12_INDEX_BUFFER_VIEW m_indexBufferView;

  private:
    Cube(const Cube&) = delete;
    Cube& operator=(const Cube&) = delete;

  };
}

