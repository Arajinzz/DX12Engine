#include "stdafx.h"
#include "Application.h"

#include "Core/DX12Device.h"
#include "Core/DX12CommandQueue.h"

namespace
{
  std::vector<UINT8> GenerateTextureData(unsigned texW, unsigned texH, unsigned pixelSize)
  {
    const UINT rowPitch = texW * pixelSize;
    const UINT cellPitch = rowPitch >> 3; // The width of a cell in the checkboard texture.
    const UINT cellHeight = texW >> 3;    // The height of a cell in the checkerboard texture.
    const UINT textureSize = rowPitch * texH;

    std::vector<UINT8> data(textureSize);
    UINT8* pData = &data[0];

    for (UINT n = 0; n < textureSize; n += pixelSize)
    {
      UINT x = n % rowPitch;
      UINT y = n / rowPitch;
      UINT i = x / cellPitch;
      UINT j = y / cellHeight;

      if (i % 2 == j % 2)
      {
        pData[n] = 0x00;        // R
        pData[n + 1] = 0x00;    // G
        pData[n + 2] = 0x00;    // B
        pData[n + 3] = 0xff;    // A
      } else
      {
        pData[n] = 0xff;        // R
        pData[n + 1] = 0xff;    // G
        pData[n + 2] = 0xff;    // B
        pData[n + 3] = 0xff;    // A
      }
    }

    return data;
  }
}

namespace Core
{
  Application::Application(UINT width, UINT height, std::wstring name)
    : DirectXApplication(width, height, name)
    , m_frameIndex(0)
    , m_beginCommandList(nullptr)
    , m_endCommandList(nullptr)
  {
  }

  void Application::OnInit()
  {
    LoadPipeline();

    // Create the first command list
    m_beginCommandList = std::make_unique<DX12CommandList>();
    
    // Create synchronization objects.
    CommandQueue().InitFence(FrameCount);

    // Create Cube, will also create pso and root signature and constant buffer for transformation
    cubes.push_back(new DX12Cube(GetWidth(), GetHeight(), 0.0));
    cubes.push_back(new DX12Cube(GetWidth(), GetHeight(), 0.5));
    cubes.push_back(new DX12Cube(GetWidth(), GetHeight(), -0.5));

    // this order is necessary to insert this command list in the end of the queue
    m_endCommandList = std::make_unique<DX12CommandList>();

    // close all commandlists
    m_beginCommandList->Close();
    m_endCommandList->Close();

    // execute commands to finish setup
    CommandQueue().ExecuteCommandLists();
    // Wait for GPU to finish Execution
    CommandQueue().WaitForGpu(m_frameIndex);
  }

  void Application::OnUpdate()
  {
    for (auto cube : cubes)
      cube->Update();
  }

  void Application::OnRender()
  {
    // Populate Command list
    // Command list allocators can only be reset when the associated 
    // command lists have finished execution on the GPU; apps should use 
    // fences to determine GPU execution progress.
    // However, when ExecuteCommandList() is called on a particular command 
    // list, that command list can then be reset at any time and must be before 
    // re-recording.
    m_beginCommandList->Reset(m_frameIndex, nullptr);
    m_endCommandList->Reset(m_frameIndex, nullptr);

    // Indicate that the back buffer will be used as a render target.
    m_beginCommandList->Transition(m_rtvHeap->GetResource(m_frameIndex), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

    // Record commands.
    const float clearColor[] = { 0.25f, 0.55f, 0.45f, 1.0f };
    m_beginCommandList->ClearDepthStencilView(m_dsvHeap->GetOffsetHandle(0));
    m_beginCommandList->ClearRenderTargetView(m_rtvHeap->GetOffsetHandle(m_frameIndex), clearColor);
    
    m_beginCommandList->Close();

    // draw cube
    for (auto cube : cubes)
      cube->Draw(m_rtvHeap.get(), m_dsvHeap.get(), m_frameIndex);

    // Indicate that the back buffer will now be used to present.
    m_endCommandList->Transition(m_rtvHeap->GetResource(m_frameIndex), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

    m_endCommandList->Close();

    CommandQueue().ExecuteCommandLists();

    // Present the frame.
    SwapChain().Present();

    MoveToNextFrame(); // try to render next frame
  }

  void Application::OnDestroy()
  {
    // Ensure that the GPU is no longer referencing resources that are about to be
    // cleaned up by the destructor.
    // Wait for the command list to execute; we are reusing the same command 
    // list in our main loop but for now, we just want to wait for setup to 
    // complete before continuing.
    // Schedule a Signal command in the queue.
    CommandQueue().WaitForGpu(m_frameIndex);
  }

  void Application::LoadPipeline()
  {
    // Create Device
    CreateDevice(); // Singleton

    // Create Command Queue
    CreateCmdQueue(); // Singleton

    // Create SwapChain
    CreateSwapChain();
    m_frameIndex = SwapChain().GetCurrentBackBufferIndex();

    // full screen transitions not supported.
    ThrowIfFailed(Factory()->MakeWindowAssociation(WindowsApplication::GetHwnd(), DXGI_MWA_NO_ALT_ENTER));

    // Create RTV heap
    m_rtvHeap = std::make_unique<DX12Heap>(FrameCount, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    // Create frame resources.
    m_rtvHeap->CreateResources();

    // Create DSV heap
    m_dsvHeap = std::make_unique<DX12Heap>(1, D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    // create resource
    m_dsvHeap->CreateResources();
  }

  void Application::MoveToNextFrame()
  {
    // Signal and increment the fence value.
    CommandQueue().SignalFence(m_frameIndex);

    // current frame Fence
    const UINT64 currentFenceValue = CommandQueue().GetFenceValue(m_frameIndex);

    // next frame
    m_frameIndex = SwapChain().GetCurrentBackBufferIndex();

    // If the next frame is not ready to be rendered yet, wait until
    CommandQueue().WaitFence(m_frameIndex);

    // How I understand it is, the current frame will always a fenceValue bigger than next frame
    // Why is that? because we begin with fence values 0 for both frames, but the current frame
    // will be increment to one.
    // so we will have an initial state of
    // currentFrame fenceValue = 1
    // nextFrame fenceValue = 0
    // so if nextFrame if available it recieves current frame fence value
    // which will be
    // currentFrame fenceValue = 1
    // nextFrame fenceValue = 2
    CommandQueue().SetFenceValue(m_frameIndex, currentFenceValue + 1);
  }
}