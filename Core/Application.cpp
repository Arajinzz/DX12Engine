#include "stdafx.h"
#include "Application.h"

#include "Core/DX12Interface.h"
#include "Core/DX12FrameResource.h"
#include "Core/DX12Skybox.h"
#include "Core/ResourceManager.h"
#include "Core/ShaderManager.h"
#include "Core/PSOManager.h"
#include "Core/SceneGraph.h"
#include "Core/RenderGraph.h"

#include <random>

#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
// Optional. define TINYOBJLOADER_USE_MAPBOX_EARCUT gives robust triangulation. Requires C++11
//#define TINYOBJLOADER_USE_MAPBOX_EARCUT
#include "tiny_obj_loader.h"

// TODO: better ERROR handling
// for example if a texture is not available like skybox
// Program exits silently without any error

namespace Core
{
  Application::Application(UINT width, UINT height, std::wstring name)
    : DirectXApplication(width, height, name)
    , m_context(nullptr)
  {
  }

  void Application::OnInit()
  {
    // things like device, context ...etc
    LoadPipeline();

    // create resource manager
    ResourceManager::Instance();

    // create shader manager
    ShaderManager::Instance();

    // create PSO manager, it has to be after shader manager
    PSOManager::Instance();

    // create render graph
    RenderGraph::Instance();

    // create scene graph
    SceneGraph::Instance();

    // TODO: maybe delete this
    CreateFrameResource();
    
    // create resources before doing anything
    FrameResource().CreateResources(m_context->GetCommandList());

    // Create Cube, will also create pso and root signature and constant buffer for transformation

    for (auto model : SceneGraph::Instance().GetModels())
      model->SetupModel(m_context->GetCommandList());

    FrameResource().GetSkybox()->Setup(m_context->GetCommandList());

    // Execute command lists
    m_context->Execute();

    // Wait for GPU to finish Execution
    m_context->WaitForGpu();
  }

  void Application::OnUpdate()
  {
    FrameResource().Update();

    for (auto model : SceneGraph::Instance().GetModels())
      model->UpdateModel();
  }

  void Application::OnRender()
  {
    // Populate Command list
    // Reset and transition to Rendering state
    m_context->PrepareForRendering(); // set heaps, rects ...etc

    // draw skybox first
    m_context->Draw(FrameResource().GetSkybox()->GetModel());

    // draw meshes
    for (auto model : SceneGraph::Instance().GetModels())
      m_context->Draw(model);

    // transition to present state
    m_context->PrepareForPresenting();

    // execute command list
    m_context->Execute();

    // Present the frame, swap chain will do that
    m_context->Present();

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

  void Application::OnResize(unsigned width, unsigned height)
  {
    if (WindowsApplication::Resizable())
    {
      // make sure that GPU is not doing anything
      m_context->WaitForGpu();

      m_width = width;
      m_height = height;

      // adapt viewport and rect in context
      m_context->Resize(width, height);
    }
  }

  void Application::OnKeyDown(UINT8 key)
  {
    float x = 0.0;
    float z = 0.0;

    if (key == 0x57)
    { // W
      z = 1.0;
    }
    else if (key == 0x41)
    { // A
      x = -1.0;
    }
    else if (key == 0x53)
    { // S
      z = -1.0;
    }
    else if (key == 0x44)
    { // D
      x = 1.0;
    }
    
    FrameResource().GetCamera()->Translate(x, z);
  }

  void Application::OnKeyUp(UINT8 key)
  {
  }

  void Application::OnMouseMove(float dx, float dy)
  {
    FrameResource().GetCamera()->Rotate(dx, dy);
  }

  void Application::LoadPipeline()
  {
    // Create DX12Interface, It will create Device and factory
    DX12Interface::Get();

    // Create Context
    // Create SwapChain, swap chain creates depth buffer and render targets
    m_context = std::make_unique<DX12Context>();

    // full screen transitions not supported.
    DX12Interface::Get().MakeWindowAssociation(WindowsApplication::GetHwnd(), DXGI_MWA_NO_ALT_ENTER);
  }
}