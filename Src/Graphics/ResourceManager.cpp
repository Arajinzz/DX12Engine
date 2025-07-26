#include "stdafx.h"
#include "ResourceManager.h"

#include "Core/DX12Interface.h"
#include "Core/DXApplicationHelper.h"

namespace Core
{
  ResourceManager::ResourceManager()
    : m_resourcesHeap(nullptr)
    , m_rtvHeap(nullptr)
    , m_dsvHeap(nullptr)
    , m_samplerHeap(nullptr)
    , m_nextFreeTex()
    , m_nextFreeMip()
    , m_nextFreeCB()
    , m_nextFreeRT()
    , m_nextFreeDS()
    , m_nextFreeSampler()
    , m_mutexTex()
    , m_mutexMip()
    , m_mutexCB()
    , m_mutexRT()
    , m_mutexDS()
    , m_mutexSampler()
  {
    m_resourcesHeap = DX12Interface::Get().CreateHeapDescriptor(
      D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, RESOURCE_HEAP_SIZE);
    // depth heap and render targets heap
    m_dsvHeap = DX12Interface::Get().CreateHeapDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, DSV_HEAP_SIZE);
    m_rtvHeap = DX12Interface::Get().CreateHeapDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, RTV_HEAP_SIZE);
    m_samplerHeap = DX12Interface::Get().CreateHeapDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, SAMPLER_HEAP_SIZE);

    // populate free Index vector
    for (unsigned i = TEX_RANGE.begin; i <= TEX_RANGE.end; i++)
      m_nextFreeTex.push_back(i);
    for (unsigned i = MIPS_RANGE.begin; i <= MIPS_RANGE.end; i++)
      m_nextFreeMip.push_back(i);
    for (unsigned i = CB_RANGE.begin; i <= CB_RANGE.end; i++)
      m_nextFreeCB.push_back(i);
    for (unsigned i = RT_RANGE.begin; i <= RT_RANGE.end; i++)
      m_nextFreeRT.push_back(i);
    for (unsigned i = DS_RANGE.begin; i <= DS_RANGE.end; i++)
      m_nextFreeDS.push_back(i);
    for (unsigned i = SAMPLER_RANGE.begin; i <= SAMPLER_RANGE.end; i++)
      m_nextFreeSampler.push_back(i);
  }

  ResourceManager::~ResourceManager()
  {
    m_resourcesHeap.Reset();
    m_dsvHeap.Reset();
    m_rtvHeap.Reset();
    m_samplerHeap.Reset();
  }

  D3D12_GPU_DESCRIPTOR_HANDLE ResourceManager::GetResourceGpuHandle(unsigned index)
  {
    return CD3DX12_GPU_DESCRIPTOR_HANDLE(
      m_resourcesHeap->GetGPUDescriptorHandleForHeapStart(),
      index,
      DX12Interface::Get().GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
  }

  D3D12_CPU_DESCRIPTOR_HANDLE ResourceManager::GetResourceCpuHandle(unsigned index)
  {
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(
      m_resourcesHeap->GetCPUDescriptorHandleForHeapStart(),
      index,
      DX12Interface::Get().GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
  }

  D3D12_GPU_DESCRIPTOR_HANDLE ResourceManager::GetRTVGpuHandle(unsigned index)
  {
    return CD3DX12_GPU_DESCRIPTOR_HANDLE(
      m_rtvHeap->GetGPUDescriptorHandleForHeapStart(),
      index,
      DX12Interface::Get().GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
  }

  D3D12_CPU_DESCRIPTOR_HANDLE ResourceManager::GetRTVCpuHandle(unsigned index)
  {
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(
      m_rtvHeap->GetCPUDescriptorHandleForHeapStart(),
      index,
      DX12Interface::Get().GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
  }

  D3D12_GPU_DESCRIPTOR_HANDLE ResourceManager::GetDSVGpuHandle(unsigned index)
  {
    return CD3DX12_GPU_DESCRIPTOR_HANDLE(
      m_dsvHeap->GetGPUDescriptorHandleForHeapStart(),
      index,
      DX12Interface::Get().GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV));
  }

  D3D12_CPU_DESCRIPTOR_HANDLE ResourceManager::GetDSVCpuHandle(unsigned index)
  {
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(
      m_dsvHeap->GetCPUDescriptorHandleForHeapStart(),
      index,
      DX12Interface::Get().GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV));
  }

  D3D12_GPU_DESCRIPTOR_HANDLE ResourceManager::GetSamplerGpuHandle(unsigned index)
  {
    return CD3DX12_GPU_DESCRIPTOR_HANDLE(
      m_samplerHeap->GetGPUDescriptorHandleForHeapStart(),
      index,
      DX12Interface::Get().GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER));
  }

  D3D12_CPU_DESCRIPTOR_HANDLE ResourceManager::GetSamplerCpuHandle(unsigned index)
  {
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(
      m_samplerHeap->GetCPUDescriptorHandleForHeapStart(),
      index,
      DX12Interface::Get().GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER));
  }

  std::unique_ptr<ResourceDescriptor> ResourceManager::CreateConstantBufferResource(size_t size, D3D12_HEAP_TYPE type)
  {
    // check if space is available
    if (m_nextFreeCB.size() < 1)
      throw std::out_of_range("[CONSTANT_BUFFER] HEAP DESCRIPTOR HAVE NO SPACE LEFT!");

    std::unique_ptr<ResourceDescriptor> output = std::make_unique<ResourceDescriptor>();
    output->resource = DX12Interface::Get().CreateConstantBuffer(size, D3D12_HEAP_TYPE_UPLOAD);
    
    {
      std::lock_guard<std::mutex> lock(m_mutexCB);
      output->index = m_nextFreeCB.front();
      m_nextFreeCB.erase(m_nextFreeCB.begin());
    }

    DX12Interface::Get().CreateConstantBufferView(output->resource.Get(), m_resourcesHeap.Get(), output->index);

    output->freeResource = [&](unsigned index) {
      std::lock_guard<std::mutex> lock(m_mutexCB);
      m_nextFreeCB.push_back(index);
    };

    return output;
  }

  std::shared_ptr<TextureDescriptor> ResourceManager::CreateTextureResource(D3D12_RESOURCE_DESC& desc, bool isCubeMap, bool generateMips)
  {
    // TODO: add condition for cubemap
    // check if space is available
    if (m_nextFreeTex.size() < 1)
      throw std::out_of_range("[TEXTURE] HEAP DESCRIPTOR HAVE NO SPACE LEFT!");

    // check if space is available
    if (m_nextFreeMip.size() < desc.MipLevels)
      throw std::out_of_range("[MIPS] HEAP DESCRIPTOR HAVE NO SPACE LEFT!");

    std::unique_ptr<TextureDescriptor> output = std::make_unique<TextureDescriptor>();
    ComPtr<ID3D12Resource> texture;
    ComPtr<ID3D12Resource> upload;
    
    {
      std::lock_guard<std::mutex> lock(m_mutexTex);
      output->index = m_nextFreeTex.front();
      // index no longer free
      m_nextFreeTex.erase(m_nextFreeTex.begin());
    }

    auto defaultProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    // texture resource
    ThrowIfFailed(DX12Interface::Get().GetDevice()->CreateCommittedResource(
      &defaultProps,
      D3D12_HEAP_FLAG_NONE,
      &desc,
      D3D12_RESOURCE_STATE_COPY_DEST,
      nullptr,
      IID_PPV_ARGS(&texture)));

    // upload heap
    auto uploadDesc = CD3DX12_RESOURCE_DESC::Buffer(
      GetRequiredIntermediateSize(texture.Get(), 0, desc.DepthOrArraySize * desc.MipLevels));
    auto uploadProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    ThrowIfFailed(DX12Interface::Get().GetDevice()->CreateCommittedResource(
      &uploadProps,
      D3D12_HEAP_FLAG_NONE,
      &uploadDesc,
      D3D12_RESOURCE_STATE_GENERIC_READ,
      nullptr,
      IID_PPV_ARGS(&upload)));

    DX12Interface::Get().CreateShaderResourceView(texture.Get(), m_resourcesHeap.Get(), output->index, isCubeMap);

    // mips?
    {
      std::lock_guard<std::mutex> lock(m_mutexMip);
      if (generateMips && m_nextFreeMip.size() >= texture->GetDesc().MipLevels)
      {
        // root signature takes 4 UAV at once
        output->mipIndex = m_nextFreeMip.front();
        output->mipLevels = texture->GetDesc().MipLevels;
        for (unsigned mip = 0; mip < texture->GetDesc().MipLevels; ++mip)
        {
          DX12Interface::Get().CreateUnorderedAccessView(texture.Get(), m_resourcesHeap.Get(), m_nextFreeMip.front(), mip);
          m_nextFreeMip.erase(m_nextFreeMip.begin());
        }
      }
    }

    output->resource = texture;
    output->upload = upload;
    output->freeResource = [&](unsigned index) {
      std::lock_guard<std::mutex> lock(m_mutexTex);
      m_nextFreeTex.push_back(index);
    };
    output->freeMips = [&](unsigned index, unsigned mips) {
      std::lock_guard<std::mutex> lock(m_mutexMip);
      for (unsigned i = index; i < index + mips; ++i)
        m_nextFreeMip.push_back(i);
    };

    return output;
  }
  std::unique_ptr<ResourceDescriptor> ResourceManager::CreateDepthResource(D3D12_RESOURCE_DESC& desc, D3D12_CLEAR_VALUE& clearValue)
  {
    // check if space is available
    if (m_nextFreeDS.size() < 1)
      throw std::out_of_range("[DEPTH] HEAP DESCRIPTOR HAVE NO SPACE LEFT!");

    // no need to check for free place I guess, I expect to have only one depth buffer
    std::unique_ptr<ResourceDescriptor> output = std::make_unique<ResourceDescriptor>();
    ComPtr<ID3D12Resource> depth;
    
    {
      std::lock_guard<std::mutex> lock(m_mutexDS);
      output->index = m_nextFreeDS.front();
      m_nextFreeDS.erase(m_nextFreeDS.begin());
    }

    auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    ThrowIfFailed(DX12Interface::Get().GetDevice()->CreateCommittedResource(
      &heapProp,
      D3D12_HEAP_FLAG_NONE,
      &desc,
      D3D12_RESOURCE_STATE_DEPTH_WRITE,
      &clearValue,
      IID_PPV_ARGS(&depth)));

    // create view
    DX12Interface::Get().CreateDepthStencilView(depth.Get(), m_dsvHeap.Get(), output->index);

    output->resource = depth;
    output->freeResource = [&](unsigned index) {
      std::lock_guard<std::mutex> lock(m_mutexDS);
      m_nextFreeDS.push_back(index);
    };

    return output;
  }

  std::shared_ptr<RenderTargetDescriptor> ResourceManager::CreateRenderTargetResource(ID3D12Resource* swapRenderTarget)
  {
    // check if space is available
    if (m_nextFreeRT.size() < 1) // TODO: this condition should be changed because we create multiple render targets per swap
      throw std::out_of_range("[RENDERTARGET] HEAP DESCRIPTOR HAVE NO SPACE LEFT!");

    // TODO: a condition for textures

    std::unique_ptr<RenderTargetDescriptor> output = std::make_unique<RenderTargetDescriptor>();

    // swap render target
    {
      std::lock_guard<std::mutex> lock(m_mutexRT);
      output->index = m_nextFreeRT.front();
      m_nextFreeRT.erase(m_nextFreeRT.begin());
      // render targets
      output->renderTargetIndex1 = m_nextFreeRT.front();
      m_nextFreeRT.erase(m_nextFreeRT.begin());
      output->renderTargetIndex2 = m_nextFreeRT.front();
      m_nextFreeRT.erase(m_nextFreeRT.begin());
    }
    DX12Interface::Get().CreateRenderTargetView(swapRenderTarget, m_rtvHeap.Get(), output->index);

    {
      std::lock_guard<std::mutex> lock(m_mutexTex);
      output->shaderResourceIndex1 = m_nextFreeTex.front();
      m_nextFreeTex.erase(m_nextFreeTex.begin());
      output->shaderResourceIndex2 = m_nextFreeTex.front();
      m_nextFreeTex.erase(m_nextFreeTex.begin());
    }

    // create resources and views
    // texture resource
    auto defaultProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    auto swapDesc = swapRenderTarget->GetDesc();
    ThrowIfFailed(DX12Interface::Get().GetDevice()->CreateCommittedResource(
      &defaultProps,
      D3D12_HEAP_FLAG_NONE,
      &swapDesc,
      D3D12_RESOURCE_STATE_COMMON,
      nullptr,
      IID_PPV_ARGS(&output->renderTarget1)));

    // texture resource
    ThrowIfFailed(DX12Interface::Get().GetDevice()->CreateCommittedResource(
      &defaultProps,
      D3D12_HEAP_FLAG_NONE,
      &swapDesc,
      D3D12_RESOURCE_STATE_COMMON,
      nullptr,
      IID_PPV_ARGS(&output->renderTarget2)));

    // create views
    DX12Interface::Get().CreateRenderTargetView(output->renderTarget1.Get(), m_rtvHeap.Get(), output->renderTargetIndex1);
    DX12Interface::Get().CreateRenderTargetView(output->renderTarget2.Get(), m_rtvHeap.Get(), output->renderTargetIndex2);
    DX12Interface::Get().CreateShaderResourceView(output->renderTarget1.Get(), m_resourcesHeap.Get(), output->shaderResourceIndex1);
    DX12Interface::Get().CreateShaderResourceView(output->renderTarget2.Get(), m_resourcesHeap.Get(), output->shaderResourceIndex2);

    // we will not store the resource because swapchain is the owner of the render target
    output->freeResource = [&](unsigned index) {
      std::lock_guard<std::mutex> lock(m_mutexRT);
      m_nextFreeRT.push_back(index);
    };

    output->freeRT = [&](unsigned RTIndex1, unsigned SRIndex1, unsigned RTIndex2, unsigned SRIndex2) {
      std::lock_guard<std::mutex> lockTex(m_mutexTex);
      std::lock_guard<std::mutex> lockRT(m_mutexRT);
      m_nextFreeRT.push_back(RTIndex1);
      m_nextFreeRT.push_back(RTIndex2);
      m_nextFreeTex.push_back(SRIndex1);
      m_nextFreeTex.push_back(SRIndex2);
    };
    // set the swap render target
    output->swapRenderTarget = swapRenderTarget;

    return output;
  }

  std::unique_ptr<Descriptor> ResourceManager::CreateSampler(D3D12_SAMPLER_DESC& desc)
  {
    // check if space is available
    if (m_nextFreeSampler.size() < 1)
      throw std::out_of_range("[SAMPLER] HEAP DESCRIPTOR HAVE NO SPACE LEFT!");

    std::unique_ptr<Descriptor> output = std::make_unique<Descriptor>();

    {
      std::lock_guard<std::mutex> lock(m_mutexSampler);
      output->index = m_nextFreeSampler.front();
      m_nextFreeSampler.erase(m_nextFreeSampler.begin());
    }

    DX12Interface::Get().CreateSampler(&desc, m_samplerHeap.Get(), output->index);

    // no need to sore resource
    output->freeResource = [&](unsigned index) {
      std::lock_guard<std::mutex> lock(m_mutexSampler);
      m_nextFreeSampler.push_back(index);
    };

    return output;
  }
}
