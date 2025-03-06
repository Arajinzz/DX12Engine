#pragma once

#include "Core/DX12CommandList.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace Core
{
  class DX12ConstantBuffer
  {
  public:
    DX12ConstantBuffer(DX12Heap* heap);
    ~DX12ConstantBuffer();

  private:
    ComPtr<ID3D12Resource> m_buffer;
    std::unique_ptr<DX12CommandList> m_commandList;

  private:
    DX12ConstantBuffer(const DX12ConstantBuffer&) = delete;
    DX12ConstantBuffer& operator=(const DX12ConstantBuffer&) = delete;
  };
}