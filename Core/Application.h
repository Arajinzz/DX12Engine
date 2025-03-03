#pragma once

#include "DirectXApplication.h"
#include "Core/DX12SwapChain.h"
#include "Core/DX12Heap.h"
#include "Core/DX12CommandList.h"
#include "Core/Cube.h"

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

    // Pipeline objects (commandQueue and Device are singletons)
    std::unique_ptr<DX12SwapChain> m_swapChain;
    // render target heap
    std::unique_ptr<DX12Heap> m_rtvHeap;
    // cube
    std::vector<Cube*> cubes;

    std::unique_ptr<DX12CommandList> m_beginCommandList;
    std::unique_ptr<DX12CommandList> m_endCommandList;

    // Synchronization objects.
    uint32_t m_frameIndex;
    

  };
}

