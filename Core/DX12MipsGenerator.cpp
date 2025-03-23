#include "stdafx.h"
#include "DX12MipsGenerator.h"

#include "Core/DXApplicationHelper.h"
#include "Core/DX12Device.h"

namespace Core
{
  DX12MipsGenerator::DX12MipsGenerator()
  {
    m_commandQueue = std::make_unique<DX12CommandQueue>(false);
    m_uavHeap = std::make_unique<DX12Heap>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    m_commandQueue->InitFence();
    //m_uavHeap->CreateHeap();

    CD3DX12_DESCRIPTOR_RANGE1 srcMip(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);
    CD3DX12_DESCRIPTOR_RANGE1 outMip(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 4, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);

    CD3DX12_ROOT_PARAMETER1 rootParameters[Core::NumRootParameters];
    rootParameters[Core::GenerateMipsCB].InitAsConstants(sizeof(GenerateMipsCB) / 4, 0);
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
    ThrowIfFailed(Device()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));

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
    Device()->CreateComputePipelineState(&computePsoDesc, IID_PPV_ARGS(&m_pipelineState));
  }

  DX12MipsGenerator::~DX12MipsGenerator()
  {
  }
}