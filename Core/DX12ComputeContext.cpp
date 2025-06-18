#include "stdafx.h"
#include "DX12ComputeContext.h"

#include "Core/DXApplicationHelper.h"
#include "Core/DX12Interface.h"
#include "Core/DX12Texture.h"
#include "Core/DX12FrameResource.h"
#include "Core/Application.h"

namespace Core
{
  DX12ComputeContext::DX12ComputeContext()
    : DX12ContextInterface()
    , m_mipsHeap(nullptr)
    , m_frameIndex(0)
    , m_commandQueue(nullptr)
    , m_commandAllocators()
    , m_commandList(nullptr)
    , m_pipelineState(nullptr)
    , m_rootSignature(nullptr)
  {
    // create heap
    m_mipsHeap = std::make_unique<DX12Heap>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    CD3DX12_DESCRIPTOR_RANGE1 srcMip(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);
    CD3DX12_DESCRIPTOR_RANGE1 outMip(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 4, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);
    CD3DX12_ROOT_PARAMETER1 rootParameters[Core::NumRootParameters];
    rootParameters[Core::GenerateMipsCB].InitAsConstants(sizeof(SGenerateMipsCB) / 4, 0);
    rootParameters[Core::SrcMip].InitAsDescriptorTable(1, &srcMip);
    rootParameters[Core::OutMip].InitAsDescriptorTable(1, &outMip);
    CD3DX12_STATIC_SAMPLER_DESC linearClampSampler(
      0,
      D3D12_FILTER_MIN_MAG_MIP_LINEAR,
      D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
      D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
      D3D12_TEXTURE_ADDRESS_MODE_CLAMP
    );
    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc(
      Core::NumRootParameters,
      rootParameters, 1, &linearClampSampler
    );
    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_1, &signature, &error));
    ThrowIfFailed(DX12Interface::Get().GetDevice()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
    Microsoft::WRL::ComPtr<ID3DBlob> shaderBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
    ThrowIfFailed(D3DCompileFromFile(L"GenerateMips_CS.hlsl", nullptr, nullptr, "main", "cs_5_1",
      D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &shaderBlob, &errorBlob));
    // Pipeline state descriptor
    D3D12_COMPUTE_PIPELINE_STATE_DESC computePsoDesc = {};
    computePsoDesc.pRootSignature = m_rootSignature.Get();  // Root signature
    computePsoDesc.CS.pShaderBytecode = shaderBlob->GetBufferPointer();
    computePsoDesc.CS.BytecodeLength = shaderBlob->GetBufferSize();
    computePsoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE; // Default flag
    // Create the compute PSO
    DX12Interface::Get().GetDevice()->CreateComputePipelineState(&computePsoDesc, IID_PPV_ARGS(&m_pipelineState));

    // create command queue
    m_commandQueue = DX12Interface::Get().CreateCommandQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE);

    // create command list and allocators for it
    for (unsigned n = 0; n < Application::FrameCount; ++n)
      m_commandAllocators.push_back(DX12Interface::Get().CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE));
    m_commandList = DX12Interface::Get().CreateCommandList(m_commandAllocators, D3D12_COMMAND_LIST_TYPE_COMPUTE);
    m_frameIndex = FrameResource().GetSwapChain()->GetCurrentBackBufferIndex();
  }

  DX12ComputeContext::~DX12ComputeContext()
  {
    m_mipsHeap.reset();
    m_commandQueue.Reset();
    m_commandAllocators.clear(); // could be a bug
    m_commandList.Reset();
    m_pipelineState.Reset();
    m_rootSignature.Reset();
  }

  void DX12ComputeContext::Execute()
  {
    // close command List
    m_commandList->Close();

    // execute commands to finish setup
    ID3D12CommandList* commandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(1, commandLists);
  }

  void DX12ComputeContext::MoveToNextFrame()
  {
    // Signal and increment the fence value.
    SignalFence(m_commandQueue.Get(), m_frameIndex);

    // current frame Fence
    const UINT64 currentFenceValue = GetFenceValue(m_frameIndex);

    // next frame
    m_frameIndex = FrameResource().GetSwapChain()->GetCurrentBackBufferIndex();

    // If the next frame is not ready to be rendered yet, wait until
    WaitFence(m_frameIndex);

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
    SetFenceValue(m_frameIndex, currentFenceValue + 1);
  }

  void DX12ComputeContext::CreateMips(DX12Mesh* mesh)
  {
    m_mipsHeap->ResetResources();
    m_mipsHeap->ResetHeap();

    //Set root signature, pso and descriptor heap
    m_commandList->SetComputeRootSignature(m_rootSignature.Get());
    m_commandList->SetPipelineState(m_pipelineState.Get());

    for (unsigned i = 0; i < mesh->GetTexturesCount(); ++i)
    {
      auto texture = mesh->GetTexture(i);
      m_mipsHeap->AddResource(texture->GetResource(), TEXTURE);
      for (unsigned mip = 0; mip < texture->GetMipsLevels(); ++mip)
      {
        m_mipsHeap->AddResource(texture->GetResource(), UAV);
      }
    }

    // heap created
    ID3D12DescriptorHeap* ppHeaps[] = { m_mipsHeap->Get() };
    m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
    for (unsigned i = 0; i < mesh->GetTexturesCount(); ++i)
    {
      auto texture = mesh->GetTexture(i);

      SGenerateMipsCB generateMipsCB;
      generateMipsCB.SrcDimension = (texture->GetResource()->GetDesc().Height & 1) << 1 | (texture->GetResource()->GetDesc().Width & 1);
      generateMipsCB.SrcMipLevel = 0;
      generateMipsCB.NumMipLevels = texture->GetMipsLevels();
      generateMipsCB.TexelSize.x = 1.0f / (float)texture->GetResource()->GetDesc().Width;
      generateMipsCB.TexelSize.y = 1.0f / (float)texture->GetResource()->GetDesc().Height;
      m_commandList->SetComputeRoot32BitConstants(0, sizeof(SGenerateMipsCB) / 4, &generateMipsCB, 0);
      auto padding = texture->GetMipsLevels() + 1;
      m_commandList->SetComputeRootDescriptorTable(1, m_mipsHeap->GetOffsetGPUHandle(i * padding));
      m_commandList->SetComputeRootDescriptorTable(2, m_mipsHeap->GetOffsetGPUHandle(i * padding + 1));

      //Dispatch the compute shader with one thread per 8x8 pixels
      m_commandList->Dispatch(max(texture->GetResource()->GetDesc().Width / 8, 1u), max(texture->GetResource()->GetDesc().Height / 8, 1u), 1);

      auto barrier1 = CD3DX12_RESOURCE_BARRIER::UAV(texture->GetResource());
      //Wait for all accesses to the destination texture UAV to be finished before generating the next mipmap, as it will be the source texture for the next mipmap
      m_commandList->ResourceBarrier(1, &barrier1);
    }
  }

  void DX12ComputeContext::WaitForGpu()
  {
    // Wait for the command list to execute; we are reusing the same command
    // list in our main loop but for now, we just want to wait for setup to
    // complete before continuing.
    // Schedule a Signal command in the queue.
    SignalFence(m_commandQueue.Get(), m_frameIndex);
    WaitFence(m_frameIndex);
  }
}