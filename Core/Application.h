#pragma once

#include "DirectXApplication.h"
#include "Core/DX12SwapChain.h"
#include "Core/DX12Heap.h"
#include "Core/DX12CommandList.h"
#include "Core/DX12Cube.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace Core
{
  class Application : public DirectXApplication
  {
  public:
    Application(UINT width, UINT height, std::wstring name);

    virtual void OnInit() override;
    virtual void OnUpdate() override;
    virtual void OnRender() override;
    virtual void OnDestroy() override;
    
    static const uint32_t FrameCount = 2;

  private:
    void LoadPipeline();
    void MoveToNextFrame();

  private:
    // render target heap
    std::unique_ptr<DX12Heap> m_rtvHeap;
    // depth heap
    std::unique_ptr<DX12Heap> m_dsvHeap;
    // cube
    std::vector<DX12Cube*> cubes;
    // command queue
    std::unique_ptr<DX12CommandQueue> m_commandQueue;
    // swap chain
    std::unique_ptr<DX12SwapChain> m_swapChain;

    // Synchronization objects.
    uint32_t m_frameIndex;
    
    CD3DX12_VIEWPORT m_viewport;
    CD3DX12_RECT m_scissorRect;

  };
}

