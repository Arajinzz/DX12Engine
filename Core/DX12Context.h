#pragma once

#include "Core/DX12CommandQueue.h"
#include "Core/DX12CommandList.h"
#include "Core/DX12Mesh.h"
#include "Core/DX12SwapChain.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace Core
{
  class DX12Context
  {
  public:
    DX12Context();
    ~DX12Context();

    void Present();
    void Execute();
    void WaitForGpu();
    void MoveToNextFrame();

    void Resize(unsigned width, unsigned height);

    void PrepareForRendering();
    void Draw(DX12Mesh* mesh);
    void PrepareForPresenting();

    DX12CommandQueue* GetCommandQueue() { return m_commandQueue.get(); }
    DX12CommandList* GetCommandList() { return m_commandList; }

  private:
    // the swapchain
    std::unique_ptr<DX12SwapChain> m_swapChain;
    // command queue
    std::unique_ptr<DX12CommandQueue> m_commandQueue;
    // command list associated to command queue
    DX12CommandList* m_commandList; // owned by command queue

    // Synchronization objects.
    uint32_t m_frameIndex;

    // Rendering viewport and rect
    CD3DX12_VIEWPORT m_viewport;
    CD3DX12_RECT m_scissorRect;

  private:
    DX12Context(const DX12Context&) = delete;
    DX12Context& operator=(const DX12Context&) = delete;

  };
}

