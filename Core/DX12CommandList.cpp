#include "stdafx.h"
#include "DX12CommandList.h"

#include "Core/DXApplicationHelper.h"
#include "Core/DX12Interface.h"
#include "Core/DX12CommandQueue.h"
#include "Core/Application.h"

namespace Core
{
  DX12CommandList::DX12CommandList(D3D12_COMMAND_LIST_TYPE type)
    : m_commandAllocators(Application::FrameCount)
    , m_type(type)
  {
    for (int n = 0; n < Application::FrameCount; ++n)
    {
      ThrowIfFailed(DX12Interface::Get().GetDevice()->CreateCommandAllocator(m_type, IID_PPV_ARGS(&m_commandAllocators[n])));
      // maybe bind this to the pso!!!!!!!!!!!
      ThrowIfFailed(DX12Interface::Get().GetDevice()->CreateCommandList(0, m_type, m_commandAllocators[n].Get(), nullptr, IID_PPV_ARGS(&m_commandList)));
    }
  }

  DX12CommandList::~DX12CommandList()
  {
    for (int n = 0; n < m_commandAllocators.size(); ++n)
      m_commandAllocators[n].Reset();
    
    m_commandList.Reset();
  }

  void DX12CommandList::Close()
  {
    ThrowIfFailed(m_commandList->Close());
  }

  void DX12CommandList::Reset(unsigned index, ID3D12PipelineState* pso)
  {
    ThrowIfFailed(m_commandAllocators[index]->Reset());
    ThrowIfFailed(m_commandList->Reset(m_commandAllocators[index].Get(), pso));
  }
  void DX12CommandList::Transition(ID3D12Resource* res, D3D12_RESOURCE_STATES from, D3D12_RESOURCE_STATES to)
  {
    auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(res, from, to);
    m_commandList->ResourceBarrier(1, &barrier1);
  }

  void DX12CommandList::ClearRenderTargetView(CD3DX12_CPU_DESCRIPTOR_HANDLE handle, const float* clearColor)
  {
    m_commandList->ClearRenderTargetView(handle, clearColor, 0, nullptr);
  }

  void DX12CommandList::ClearDepthStencilView(CD3DX12_CPU_DESCRIPTOR_HANDLE handle)
  {
    m_commandList->ClearDepthStencilView(
      handle,
      D3D12_CLEAR_FLAG_DEPTH,
      1.0f,    // Clear depth to maximum (far plane)
      0,       // Clear stencil to 0
      0, nullptr
    );
  }

  void DX12CommandList::SetRootSignature(ID3D12RootSignature* sig)
  {
    m_commandList->SetGraphicsRootSignature(sig);
  }

  void DX12CommandList::SetDescriptorHeap(DX12Heap* heap)
  {

    ID3D12DescriptorHeap* ppHeaps[] = { heap->Get() };
    m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
  }
}