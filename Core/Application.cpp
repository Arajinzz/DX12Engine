#include "stdafx.h"
#include "Application.h"

#include "Core/DX12Device.h"
#include "Core/DX12CommandQueue.h"
#include "Core/DX12FrameResource.h"

namespace Core
{
  Application::Application(UINT width, UINT height, std::wstring name)
    : DirectXApplication(width, height, name)
    , m_context(nullptr)
  {
  }

  void Application::OnInit()
  {
    LoadPipeline();

    CreateFrameResource();

    // Create Cube, will also create pso and root signature and constant buffer for transformation
    cubes.push_back(new DX12Cube(GetWidth(), GetHeight(), 0.0));
    cubes.push_back(new DX12Cube(GetWidth(), GetHeight(), 0.5));
    cubes.push_back(new DX12Cube(GetWidth(), GetHeight(), -0.5));

    FrameResource().Init(m_context->GetCommandList());

    // Execute command lists
    m_context->Execute();

    // Wait for GPU to finish Execution
    m_context->WaitForGpu();
  }

  void Application::OnUpdate()
  {
    FrameResource().Update();

    for (auto cube : cubes)
      cube->Update();
  }

  void Application::OnRender()
  {
    // Populate Command list
    // Reset and transition to Rendering state
    m_context->PrepareForRendering(); // set heaps, rects ...etc

    // draw cube
    for (auto cube : cubes)
    {
      m_context->Draw(cube);
    }

    // transition to present state
    m_context->PrepareForPresenting();

    // execute command list
    m_context->Execute();

    // Present the frame.
    SwapChain().Present();

    m_context->MoveToNextFrame(); // try to render next frame
  }

  void Application::OnDestroy()
  {
    // Ensure that the GPU is no longer referencing resources that are about to be
    // cleaned up by the destructor.
    // Wait for the command list to execute; we are reusing the same command 
    // list in our main loop but for now, we just want to wait for setup to 
    // complete before continuing.
    // Schedule a Signal command in the queue.
    m_context->WaitForGpu();
  }

  void Application::LoadPipeline()
  {
    // Create Device
    CreateDevice(); // Singleton

    // Create Context
    m_context = std::make_unique<DX12Context>();

    // Create SwapChain, swap chain creates depth buffer and render targets
    CreateSwapChain(m_context->GetCommandQueue());

    // Init Context
    m_context->Init();

    // full screen transitions not supported.
    ThrowIfFailed(Factory()->MakeWindowAssociation(WindowsApplication::GetHwnd(), DXGI_MWA_NO_ALT_ENTER));
  }
}