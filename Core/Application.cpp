#include "stdafx.h"
#include "Application.h"

#include "Core/DX12Interface.h"
#include "Core/DX12CommandQueue.h"
#include "Core/DX12FrameResource.h"
#include "Core/DX12Skybox.h"

#include <random>

namespace Core
{
  Application::Application(UINT width, UINT height, std::wstring name)
    : DirectXApplication(width, height, name)
    , m_context(nullptr)
    , m_triangleCount(0)
    , m_meshes()
  {
  }

  void Application::OnInit()
  {
    LoadPipeline();

    CreateFrameResource();
    
    // create resources before doing anything
    FrameResource().CreateResources(m_context->GetCommandList());

    // Create Cube, will also create pso and root signature and constant buffer for transformation
    auto modelNumber = 1;
    for (int i = 0; i < modelNumber; ++i)
      m_meshes.push_back(new DX12Mesh());

    std::random_device rd;  // Seed from hardware
    std::mt19937 gen(rd()); // Mersenne Twister PRNG
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f); // Range [0,1]

    for (auto mesh : m_meshes)
    {
      //XMFLOAT3 translation = {5 * dist(gen), 5 * dist(gen), 5 * dist(gen) };
      mesh->LoadMesh("models\\sponza.obj");
      mesh->SetupMesh(m_context->GetCommandList()->Get());
      //mesh->SetTranslation(translation);
      m_triangleCount += mesh->GetTriangleCount();
      // create mip maps
      m_context->CreateMips(mesh);
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

    for (auto mesh: m_meshes)
      mesh->UpdateMesh();
  }

  void Application::OnRender()
  {
    // Populate Command list
    // Reset and transition to Rendering state
    m_context->PrepareForRendering(); // set heaps, rects ...etc

    // draw skybox first
    m_context->Draw(FrameResource().GetSkybox()->GetMesh());

    // draw meshes
    for (auto mesh : m_meshes)
      m_context->Draw(mesh);

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
    ThrowIfFailed(DX12Interface::Get().GetFactory()->MakeWindowAssociation(WindowsApplication::GetHwnd(), DXGI_MWA_NO_ALT_ENTER));
  }
}