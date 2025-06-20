#include "stdafx.h"
#include "DX12Texture.h"

#include "Core/DX12Interface.h"
#include "Core/DXApplicationHelper.h"
#include "Core/DX12FrameResource.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace Core
{
  DX12Texture::DX12Texture(std::vector<std::string> paths, unsigned mips)
    : m_imgPtrs()
    , m_metaData()
    , m_mipsLevels(mips)
    , m_texture()
    , m_upload()
    , m_mips()
  {
    for (const auto& path : paths)
    {
      MetaData metadata;
      m_imgPtrs.emplace_back(stbi_load(path.c_str(), &metadata.width, &metadata.height, &metadata.channels, 4));
      metadata.channels = 4; // force to 4
      m_metaData.push_back(metadata);
    }

    D3D12_RESOURCE_DESC textureDesc = {};
    textureDesc.MipLevels = m_mipsLevels;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.Width = m_metaData[0].width;
    textureDesc.Height = m_metaData[0].height;
    textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    textureDesc.DepthOrArraySize = static_cast<unsigned>(m_imgPtrs.size());
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

    auto defaultProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    m_texture = ResourceManager::Instance().CreateTextureResource(
      textureDesc, defaultProps, D3D12_RESOURCE_STATE_COPY_DEST, true); // yes create view

    auto uploadDesc = CD3DX12_RESOURCE_DESC::Buffer(
      GetRequiredIntermediateSize(m_texture.resource.Get(), 0, textureDesc.DepthOrArraySize * textureDesc.MipLevels));
    auto uploadProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    m_upload = ResourceManager::Instance().CreateTextureResource(
      uploadDesc, uploadProps, D3D12_RESOURCE_STATE_GENERIC_READ, false); // don't create view

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

  }

  DX12Texture::~DX12Texture()
  {
  }

  void DX12Texture::CopyToGPU(ID3D12GraphicsCommandList* commandList)
  {
    // Copy data to the intermediate upload heap and then schedule 
    // a copy from the upload heap to the diffuse texture.
    std::vector<D3D12_SUBRESOURCE_DATA> textureData(m_imgPtrs.size());

    for (unsigned i = 0; i < m_imgPtrs.size(); ++i)
    {
      textureData[i].pData = m_imgPtrs[i];
      textureData[i].RowPitch = m_metaData[i].width * m_metaData[i].channels; // width * pixelSize
      textureData[i].SlicePitch = textureData[i].RowPitch * m_metaData[i].height; // rowpitch * height
    }

    const UINT subResourceCount = static_cast<unsigned>(1 * m_metaData.size());
    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
      m_texture.resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    UpdateSubresources(commandList, m_texture.resource.Get(), m_upload.resource.Get(), 0, 0, subResourceCount, textureData.data());
    commandList->ResourceBarrier(1, &barrier);

    // free
    for (auto ptr : m_imgPtrs)
    {
      stbi_image_free(ptr);
    }
  }

  void DX12Texture::GenerateMips(ID3D12GraphicsCommandList* commandList)
  {
    //Set root signature, pso and descriptor heap
    commandList->SetComputeRootSignature(m_rootSignature.Get());
    commandList->SetPipelineState(m_pipelineState.Get());
    
    m_mips = ResourceManager::Instance().CreateMipsForTexture(m_texture);

    // heap created
    ID3D12DescriptorHeap* ppHeaps[] = { ResourceManager::Instance().GetTexHeap(), ResourceManager::Instance().GetMipsHeap() };
    commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

    //Transition from pixel shader resource to unordered access
    auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(
      m_texture.resource.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    commandList->ResourceBarrier(1, &barrier1);

    SGenerateMipsCB generateMipsCB;
    generateMipsCB.SrcDimension = (m_texture.resource->GetDesc().Height & 1) << 1 | (m_texture.resource->GetDesc().Width & 1);
    generateMipsCB.SrcMipLevel = 0;
    generateMipsCB.NumMipLevels = m_mipsLevels;
    generateMipsCB.TexelSize.x = 1.0f / (float)m_texture.resource->GetDesc().Width;
    generateMipsCB.TexelSize.y = 1.0f / (float)m_texture.resource->GetDesc().Height;
    commandList->SetComputeRoot32BitConstants(0, sizeof(SGenerateMipsCB) / 4, &generateMipsCB, 0);
    commandList->SetComputeRootDescriptorTable(1, m_texture.gpuHandle);
    commandList->SetComputeRootDescriptorTable(2, m_mips.gpuHandle);

    //Dispatch the compute shader with one thread per 8x8 pixels
    commandList->Dispatch(
      max(static_cast<unsigned>(m_texture.resource->GetDesc().Width / 8u), 1u), max(static_cast<unsigned>(m_texture.resource->GetDesc().Height / 8u), 1u), 1);

    barrier1 = CD3DX12_RESOURCE_BARRIER::UAV(m_texture.resource.Get());
    //Wait for all accesses to the destination texture UAV to be finished before generating the next mipmap, as it will be the source texture for the next mipmap
    commandList->ResourceBarrier(1, &barrier1);

    barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(
      m_texture.resource.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    commandList->ResourceBarrier(1, &barrier1);
  }
}
