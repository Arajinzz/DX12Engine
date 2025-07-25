#include "stdafx.h"
#include "DX12Context.h"

#include "Core/WindowsApplication.h"
#include "Core/Application.h"
#include "Core/DX12Interface.h"
#include "Core/DX12Texture.h"
#include "Core/ResourceManager.h"

// helpers
namespace
{
  std::unique_ptr<Core::ResourceDescriptor> CreateDepthResource(unsigned width, unsigned height)
  {
    // Create resouce
    D3D12_RESOURCE_DESC depthStencilDesc = {};
    depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthStencilDesc.Alignment = 0;
    depthStencilDesc.Width = width;  // Swap chain width
    depthStencilDesc.Height = height; // Swap chain height
    depthStencilDesc.DepthOrArraySize = 1;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthStencilDesc.SampleDesc.Count = 1; // No multi-sampling
    depthStencilDesc.SampleDesc.Quality = 0;
    depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = DXGI_FORMAT_D32_FLOAT;
    clearValue.DepthStencil.Depth = 1.0f;
    clearValue.DepthStencil.Stencil = 0;

    return Core::ResourceManager::Instance().CreateDepthResource(depthStencilDesc, clearValue);
  }
}

namespace Core
{
  DX12Context::DX12Context()
    : m_frameIndex(0)
    , m_commandQueue(nullptr)
    , m_commandList(nullptr)
    , m_commandAllocators()
    , m_fence(nullptr)
    , m_fenceEvent()
    , m_fenceValues()
    , m_swapChain(nullptr)
    , m_renderTargets()
    , m_depth()
  {
    RECT rect;
    GetClientRect(WindowsApplication::GetHwnd(), &rect);
    auto width = rect.right - rect.left;
    auto height = rect.bottom - rect.top;
    m_viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
    m_scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(width), static_cast<LONG>(height));

    // create command queue
    m_commandQueue = DX12Interface::Get().CreateCommandQueue();

    // create command list and allocators for it
    for (unsigned n = 0; n < Application::FrameCount; ++n)
      m_commandAllocators.push_back(DX12Interface::Get().CreateCommandAllocator());
    m_commandList = DX12Interface::Get().CreateCommandList(m_commandAllocators);

    // Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = Application::FrameCount;
    swapChainDesc.Width = rect.right - rect.left;
    swapChainDesc.Height = rect.bottom - rect.top;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> swapChain = DX12Interface::Get().CreateSwapChainForHwnd(
      swapChainDesc,
      WindowsApplication::GetHwnd(),
      m_commandQueue.Get() // Swap chain needs the queue so that it can force a flush on it.
    );
    ThrowIfFailed(swapChain.As(&m_swapChain));

    // Add render targets
    for (unsigned i = 0; i < Application::FrameCount; ++i)
    {
      ComPtr<ID3D12Resource> renderTarget;
      ThrowIfFailed(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTarget)));
      m_renderTargets.push_back(ResourceManager::Instance().CreateRenderTargetResource(renderTarget.Get()));
    }

    // create depth resource
    m_depth = CreateDepthResource(swapChainDesc.Width, swapChainDesc.Height);

    // Create synchronization objects.
    InitFence();
  }

  DX12Context::~DX12Context()
  {
    m_commandList.Reset();
    m_fence.Reset();
    m_fenceValues.clear();
    m_commandQueue.Reset();
    m_commandAllocators.clear(); // could be a bug
    m_swapChain.Reset();
    m_renderTargets.clear();
    m_depth.reset();
  }

  void DX12Context::Present()
  {
    // no vsync
    ThrowIfFailed(m_swapChain->Present(0, 0));
  }

  void DX12Context::Execute()
  {
    // close command List
    m_commandList->Close();

    // execute commands to finish setup
    ID3D12CommandList* commandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(1, commandLists);
  }

  void DX12Context::WaitForGpu()
  {
    // Wait for the command list to execute; we are reusing the same command
    // list in our main loop but for now, we just want to wait for setup to
    // complete before continuing.
    // Schedule a Signal command in the queue.
    SignalFence();
    WaitFence();
  }

  void DX12Context::MoveToNextFrame()
  {
    // Signal and increment the fence value.
    ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_fenceValues[m_frameIndex]));

    // current frame Fence
    const UINT64 currentFenceValue = m_fenceValues[m_frameIndex];

    // next frame
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    // If the next frame is not ready to be rendered yet, wait until
    WaitFence();

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
    m_fenceValues[m_frameIndex] = currentFenceValue + 1;
  }

  void DX12Context::Resize(unsigned width, unsigned height)
  {
    // delete old resources
    m_renderTargets.clear();
    m_depth.reset();

    // resize swapchain buffers
    auto hr = m_swapChain->ResizeBuffers(
      Application::FrameCount, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);

    // Add render targets
    for (unsigned i = 0; i < Application::FrameCount; ++i)
    {
      ComPtr<ID3D12Resource> renderTarget;
      ThrowIfFailed(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTarget)));
      m_renderTargets.push_back(ResourceManager::Instance().CreateRenderTargetResource(renderTarget.Get()));
    }
    
    // recreate depth
    m_depth = CreateDepthResource(width, height);

    // initialize rects
    m_viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
    m_scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(width), static_cast<LONG>(height));

    // initialize current frame
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
    
    // reset fence
    m_fence.Reset();
    m_fenceValues.clear();

    // initialize fence again
    InitFence();
  }

  void DX12Context::BeginFrame()
  {
    // Command list allocators can only be reset when the associated 
    // command lists have finished execution on the GPU; apps should use 
    // fences to determine GPU execution progress.
    // However, when ExecuteCommandList() is called on a particular command 
    // list, that command list can then be reset at any time and must be before 
    // re-recording.
    ThrowIfFailed(m_commandAllocators[m_frameIndex]->Reset());
    ThrowIfFailed(m_commandList->Reset(m_commandAllocators[m_frameIndex].Get(), nullptr));

    // get render target resource
    std::shared_ptr<RenderTargetDescriptor> renderTarget = m_renderTargets[m_frameIndex];
    // Indicate that the back buffer will be used as a render target.
    auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(
      renderTarget->swapRenderTarget, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    m_commandList->ResourceBarrier(1, &barrier1);

    // these must be done in the same commandlist as drawing
    // because they set a state for rendering
    // and states they reset between command lists
    // since we are using bundles for drawing this should be fine
    m_commandList->RSSetViewports(1, &m_viewport);
    m_commandList->RSSetScissorRects(1, &m_scissorRect);

    // set the active render target
    renderTarget->SwapActive();
    auto barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(
      renderTarget->activeRT, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);
    m_commandList->ResourceBarrier(1, &barrier2);
    // set the last render target to pixel shader resource, to accumulate the previous color
    if (renderTarget->lastRT)
    {
      auto barrier3 = CD3DX12_RESOURCE_BARRIER::Transition(
        renderTarget->lastRT, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
      m_commandList->ResourceBarrier(1, &barrier3);
    }

    // set render target and depth buffer
    auto rtvHandle = ResourceManager::Instance().GetRTVCpuHandle(renderTarget->activeRTIndex);
    auto dsvHandle = ResourceManager::Instance().GetDSVCpuHandle(0);
    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
    const float clearColor[] = { 0.25f, 0.55f, 0.45f, 1.0f };
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    m_commandList->ClearDepthStencilView(
      dsvHandle,
      D3D12_CLEAR_FLAG_DEPTH,
      1.0f,    // Clear depth to maximum (far plane)
      0,       // Clear stencil to 0
      0, nullptr
    );
  }

  void DX12Context::Draw(DX12Model* model, ID3D12PipelineState* pso, ID3D12RootSignature* rootSig)
  {
    model->DrawModel(pso, rootSig, m_commandList.Get());
  }

  void DX12Context::EndFrame()
  {
    // get render target resource
    std::shared_ptr<RenderTargetDescriptor> renderTarget = m_renderTargets[m_frameIndex];
    // Indicate that the back buffer will now be used to present.
    auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(
      renderTarget->swapRenderTarget, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    m_commandList->ResourceBarrier(1, &barrier1);
    // set the active render target
    auto barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(
      renderTarget->activeRT, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);
    m_commandList->ResourceBarrier(1, &barrier2);
    // set the last render target to pixel shader resource, to accumulate the previous color
    if (renderTarget->lastRT)
    {
      auto barrier3 = CD3DX12_RESOURCE_BARRIER::Transition(
        renderTarget->lastRT, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON);
      m_commandList->ResourceBarrier(1, &barrier3);
    }
  }

  void DX12Context::InitFence()
  {
    m_fence = DX12Interface::Get().CreateFence();
    for (unsigned n = 0; n < Application::FrameCount; ++n)
      m_fenceValues.push_back(0);
    m_fenceValues[0]++; // start from buffer number 0

    m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (m_fenceEvent == nullptr)
      ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));

    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
  }

  void DX12Context::SignalFence()
  {
    ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_fenceValues[m_frameIndex]));
  }

  void DX12Context::WaitFence()
  {
    // gpu didn't finish yet
    auto completedValue = m_fence->GetCompletedValue();

    if (completedValue < m_fenceValues[m_frameIndex])
    {
      // Wait until the fence has been processed.
      ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent));
      WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
    }

    // Increment the fence value for the current frame.
    m_fenceValues[m_frameIndex]++;
  }

}