#include "stdafx.h"
#include "Application.h"

#include "Core/DX12Device.h"
#include "Core/DX12CommandQueue.h"

namespace Core
{
  Application::Application(UINT width, UINT height, std::wstring name)
    : DirectXApplication(width, height, name)
    , m_frameIndex(0)
    , m_swapChain(nullptr)
    , m_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height))
    , m_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height))
  {
  }

  void Application::OnInit()
  {
    LoadPipeline();

    // Create the command list.
    m_commandList = std::make_unique<DX12CommandList>(FrameCount);
    // Create synchronization objects.
    CommandQueue().InitFence(FrameCount);


    // Create root signature
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
    // This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1; // my gpu should support it
    CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
    CD3DX12_ROOT_PARAMETER1 rootParams[1];
    // init
    ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
    rootParams[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_VERTEX);

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init_1_1(_countof(rootParams), rootParams, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
    ThrowIfFailed(Device()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
    
    ComPtr<ID3DBlob> vertexShader;
    ComPtr<ID3DBlob> pixelShader;
    ComPtr<ID3DBlob> errorBlob;
    // Enable better shader debugging with the graphics debugging tools.
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
    ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"shaders.hlsl").c_str(), nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, &errorBlob));
    ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"shaders.hlsl").c_str(), nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));
    // Define the vertex input layout.
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    // Describe and create the graphics pipeline state object (PSO).
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
    psoDesc.pRootSignature = m_rootSignature.Get();
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;
    ThrowIfFailed(Device()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));



    // Create Cube
    cubes.push_back(new Cube(m_commandList->Get()));




    {
      // Map and initialize the constant buffer. We don't unmap this until the
      // app closes. Keeping things mapped for the lifetime of the resource is okay.
      CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
      ThrowIfFailed(m_cbvHeap->GetResource(0)->Map(0, &readRange, reinterpret_cast<void**>(&m_pCbvDataBegin)));
      memcpy(m_pCbvDataBegin, &m_constantBufferData, sizeof(m_constantBufferData));
    }

    // Close the command list and execute it to begin the initial GPU setup.
    m_commandList->Close();
    // execute commands to finish setup
    CommandQueue().ExecuteCommandLists();
    // Wait for GPU to finish Execution
    CommandQueue().WaitForGpu(m_frameIndex);
  }

  void Application::OnUpdate()
  {
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
    m_commandList->Reset(m_frameIndex, m_pipelineState.Get());

    // Set necessary state.
    m_commandList->SetRootSignature(m_rootSignature.Get());
    m_commandList->SetDescriptorHeap(m_cbvHeap.get());
    
    auto handle = m_cbvHeap->Get()->GetGPUDescriptorHandleForHeapStart();
    m_commandList->Get()->SetGraphicsRootDescriptorTable(0, handle);
    m_commandList->Get()->RSSetViewports(1, &m_viewport);
    m_commandList->Get()->RSSetScissorRects(1, &m_scissorRect);

    // Indicate that the back buffer will be used as a render target.
    m_commandList->Transition(m_rtvHeap->GetResource(m_frameIndex), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    
    auto rtvhandle = m_rtvHeap->GetOffsetHandle(m_frameIndex);
    m_commandList->Get()->OMSetRenderTargets(1, &rtvhandle, FALSE, nullptr);

    // Record commands.
    const float clearColor[] = { 0.25f, 0.25f, 0.45f, 1.0f };
    m_commandList->ClearRenderTargetView(m_rtvHeap->GetOffsetHandle(m_frameIndex), clearColor);

    // draw cube
    for (auto cube : cubes)
    { // has commandlist as reference
      cube->Draw();
    }

    // Indicate that the back buffer will now be used to present.
    m_commandList->Transition(m_rtvHeap->GetResource(m_frameIndex), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

    m_commandList->Close();
    CommandQueue().ExecuteCommandLists();

    // Present the frame.
    m_swapChain->Present();

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
    m_swapChain = std::make_unique<DX12SwapChain>(FrameCount, m_width, m_height);
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    // full screen transitions not supported.
    ThrowIfFailed(Factory()->MakeWindowAssociation(WindowsApplication::GetHwnd(), DXGI_MWA_NO_ALT_ENTER));

    // Create RTV heap
    m_rtvHeap = std::make_unique<DX12Heap>(FrameCount, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, m_swapChain.get());
    // Create frame resources.
    m_rtvHeap->CreateResources();

    // constant buffer for root signature
    m_cbvHeap = std::make_unique<DX12Heap>(1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, m_swapChain.get());
    m_cbvHeap->CreateResources();
  }

  void Application::MoveToNextFrame()
  {
    // Signal and increment the fence value.
    CommandQueue().SignalFence(m_frameIndex);

    // current frame Fence
    const UINT64 currentFenceValue = CommandQueue().GetFenceValue(m_frameIndex);

    // next frame
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

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