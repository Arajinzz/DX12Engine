#pragma once

#include "Core/DX12Heap.h"
#include "Core/DX12Mesh.h"
#include "Core/DX12SwapChain.h"
#include "Core/DX12ContextInterface.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace Core
{
  class DX12GraphicsContext : public DX12ContextInterface
  {
  public:
    DX12GraphicsContext();
    ~DX12GraphicsContext();

    void Present();
    void Execute();
    void MoveToNextFrame();
    void Resize(unsigned width, unsigned height);

    void Reset();
    void PrepareForRendering();
    void Draw(DX12Mesh* mesh);
    // HACK
    void TransitionTextures(DX12Mesh* mesh);
    void PrepareForPresenting();
    void WaitForGpu();

    ID3D12CommandQueue* GetCommandQueue() { return m_commandQueue.Get(); }
    ID3D12GraphicsCommandList* GetCommandList() { return m_commandList.Get(); }

  private:  
    // command queue
    ComPtr<ID3D12CommandQueue> m_commandQueue;
    // command list
    std::vector<ComPtr<ID3D12CommandAllocator>> m_commandAllocators;
    ComPtr<ID3D12GraphicsCommandList> m_commandList;

    // Synchronization objects.
    uint32_t m_frameIndex;

    // Rendering viewport and rect
    CD3DX12_VIEWPORT m_viewport;
    CD3DX12_RECT m_scissorRect;

  private:
    DX12GraphicsContext(const DX12GraphicsContext&) = delete;
    DX12GraphicsContext& operator=(const DX12GraphicsContext&) = delete;

  };
}

