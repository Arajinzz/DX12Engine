#pragma once

#include "Core/DirectXApplication.h"
#include "Core/DX12Model.h"
#include "Core/DX12Context.h"
#include "Core/DX12Mesh.h"

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
 
    static const uint32_t FrameCount = 2;

  private:
    void LoadPipeline();

    unsigned m_triangleCount;

  private:
    // main context
    std::unique_ptr<DX12Context> m_context;

    // meshes
    std::vector<DX12Mesh*> m_meshes;

  };
}

