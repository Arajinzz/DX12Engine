#pragma once

#include "DirectXApplication.h"
#include "Core/DX12SwapChain.h"
#include "Core/DX12Heap.h"
#include "Core/DX12CommandList.h"

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
    void MoveToNextFrame();

  private:
    static const uint32_t FrameCount = 2;

    // Pipeline objects (commandQueue and Device are singletons)
    std::unique_ptr<DX12SwapChain> m_swapChain;
    // render target heap
    std::unique_ptr<DX12Heap> m_rtvHeap;
    // command List unique_ptr???
    std::unique_ptr<DX12CommandList> m_commandList;

    // Synchronization objects.
    uint32_t m_frameIndex;
    

  };
}

