#pragma once

#include "DirectXApplication.h"
#include "Core/DX12SwapChain.h"
#include "Core/DX12Heap.h"
#include "Core/DX12CommandList.h"
#include "Core/DX12Cube.h"
#include "Core/DX12Context.h"

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

  private:
    // main context
    std::unique_ptr<DX12Context> m_context;

    // cube
    std::vector<DX12Cube*> cubes;

  };
}

