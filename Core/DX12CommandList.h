#pragma once

using namespace DirectX;
using Microsoft::WRL::ComPtr;

// GRAPHICS COMMAND LIST

namespace Core
{
  class DX12CommandList
  {
  public:
    DX12CommandList(unsigned int allocators);
    ~DX12CommandList();

    void Close();
    void Reset(unsigned index);
    void Transition(ID3D12Resource* res, D3D12_RESOURCE_STATES from, D3D12_RESOURCE_STATES to);
    void ClearRenderTargetView(CD3DX12_CPU_DESCRIPTOR_HANDLE handle, const float* clearColor);

  private:
    DX12CommandList(const DX12CommandList&) = delete;
    DX12CommandList& operator=(const DX12CommandList&) = delete;

  private:
    std::vector<ComPtr<ID3D12CommandAllocator>> m_commandAllocators;
    ComPtr<ID3D12GraphicsCommandList> m_commandList;
  };
}

