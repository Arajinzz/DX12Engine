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

  private:
    void LoadPipeline();
    void MoveToNextFrame();

  private:
    static const uint32_t FrameCount = 2;

    // Pipeline objects (commandQueue and Device are singletons)
    std::unique_ptr<DX12SwapChain> m_swapChain;
    // render target heap
    std::unique_ptr<DX12Heap> m_rtvHeap;
    // cube
    std::vector<Cube*> cubes;

    CD3DX12_VIEWPORT m_viewport;
    CD3DX12_RECT m_scissorRect;

    // command List unique_ptr???
    std::unique_ptr<DX12CommandList> m_commandList;

    // Synchronization objects.
    uint32_t m_frameIndex;
    

  };
}

