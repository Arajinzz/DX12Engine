#include "stdafx.h"
#include "Application.h"

#include "Core/DX12Interface.h"
#include "Core/DX12FrameResource.h"
#include "Core/DX12Skybox.h"
#include "Core/ResourceManager.h"
#include "Core/ShaderManager.h"

#include <random>

namespace Core
{
  Application::Application(UINT width, UINT height, std::wstring name)
    : DirectXApplication(width, height, name)
    , m_context(nullptr)
    , m_triangleCount(0)
    , m_models()
  {
  }

  void Application::OnInit()
  {
    LoadPipeline();

    //assert(1 == 0);

    // create resource manager
    ResourceManager::Instance();

    // create shader manager
    ShaderManager::Instance();

    CreateFrameResource();
    
    // create resources before doing anything
    FrameResource().CreateResources(m_context->GetCommandList());

    // Create Cube, will also create pso and root signature and constant buffer for transformation
    auto modelNumber = 1;
    for (int i = 0; i < modelNumber; ++i)
      m_models.push_back(new DX12Model());

    for (auto model : m_models)
    {
      model->LoadModel("models\\sponza.obj");
      model->SetupModel(m_context->GetCommandList());
      //mesh->SetTranslation(translation);
      m_triangleCount += model->GetTriangleCount();
    }

    FrameResource().GetSkybox()->Setup(m_context->GetCommandList());

    // Execute command lists
    m_context->Execute();

    // Wait for GPU to finish Execution
    m_context->WaitForGpu();
  }

  void Application::OnUpdate()
  {
    FrameResource().Update();

    for (auto model: m_models)
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
    for (auto model : m_models)
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
    // delete models
    for (auto model : m_models)
      delete model;
    m_models.clear();

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