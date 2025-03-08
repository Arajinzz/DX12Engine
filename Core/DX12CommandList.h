#pragma once

#include "Core/DX12Heap.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

// GRAPHICS COMMAND LIST

namespace Core
{
  class DX12CommandList
  {
  public:
    DX12CommandList(D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT);
    ~DX12CommandList();

    ID3D12GraphicsCommandList* Get() { return m_commandList.Get(); }

    void Close();
    void Reset(unsigned index, ID3D12PipelineState* pso);
    void Transition(ID3D12Resource* res, D3D12_RESOURCE_STATES from, D3D12_RESOURCE_STATES to);
    void ClearRenderTargetView(CD3DX12_CPU_DESCRIPTOR_HANDLE handle, const float* clearColor);
    void ClearDepthStencilView(CD3DX12_CPU_DESCRIPTOR_HANDLE handle);
    void SetRootSignature(ID3D12RootSignature* sig);
    void SetDescriptorHeap(DX12Heap* heap);

  private:
    DX12CommandList(const DX12CommandList&) = delete;
    DX12CommandList& operator=(const DX12CommandList&) = delete;

  private:
    std::vector<ComPtr<ID3D12CommandAllocator>> m_commandAllocators;
    ComPtr<ID3D12GraphicsCommandList> m_commandList;
    D3D12_COMMAND_LIST_TYPE m_type;
  };
}

