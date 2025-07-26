#pragma once

#include "Core/DirectXApplication.h"

#include "Scene/DX12Model.h"

#include "Graphics/DX12Context.h"

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
    virtual void OnResize(unsigned width, unsigned height) override;

    virtual void OnKeyDown(UINT8 key) override;
    virtual void OnKeyUp(UINT8 key) override;
    virtual void OnMouseMove(float dx, float dy) override;
 
    static const uint32_t FrameCount = 2;

  private:
    // main context
    std::unique_ptr<Graphics::DX12Context> m_context;

  };
}

