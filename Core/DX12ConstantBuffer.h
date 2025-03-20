#pragma once

#include "Core/DX12CommandList.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace Core
{
  class DX12ConstantBuffer
  {
  public:
    DX12ConstantBuffer();
    ~DX12ConstantBuffer();

    void SetModel(XMMATRIX model);
    void SetView(XMMATRIX view);
    void SetProjection(XMMATRIX proj);

  private:
    // temporary
    struct ConstantBufferData
    {
      XMMATRIX model;
      XMMATRIX view;
      XMMATRIX projection;
      float padding[16]; // Padding so the constant buffer is 256-byte aligned.
    };

    ComPtr<ID3D12Resource> m_buffer;

    // data
    ConstantBufferData m_constantBufferData;
    UINT8* m_pCbvDataBegin;

  private:
    DX12ConstantBuffer(const DX12ConstantBuffer&) = delete;
    DX12ConstantBuffer& operator=(const DX12ConstantBuffer&) = delete;
  };
}