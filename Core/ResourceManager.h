#pragma once

#include <mutex>
#include <functional>

using namespace DirectX;
using Microsoft::WRL::ComPtr;

// Regions
// 0 - 999 : textures
// 1000 - 4999 : mips [expecting 4 mips per texture]
// 5000 - 7999 : constant buffers

namespace Core
{
  struct Range
  {
    unsigned begin;
    unsigned end;
  };

  struct Descriptor
  {
    unsigned index;
    std::function<void(unsigned)> freeResource;

    virtual ~Descriptor()
    {
      if (freeResource)
        freeResource(index);
    }
  };

  struct ResourceDescriptor : public Descriptor
  {
    ComPtr<ID3D12Resource> resource;
    
    virtual ~ResourceDescriptor()
    {
      resource.Reset();
    }
  };

  struct TextureDescriptor : public ResourceDescriptor
  {
    ComPtr<ID3D12Resource> upload;
    unsigned mipIndex;
    unsigned mipLevels;
    std::function<void(unsigned, unsigned)> freeMips;

    ~TextureDescriptor()
    {
      upload.Reset();
      if (freeMips)
        freeMips(mipIndex, mipLevels);
    }
  };

  struct RenderTargetDescriptor : public Descriptor
  {
    unsigned renderTargetIndex1;
    unsigned shaderResourceIndex1;
    ComPtr<ID3D12Resource> renderTarget1;
    unsigned renderTargetIndex2;
    unsigned shaderResourceIndex2;
    ComPtr<ID3D12Resource> renderTarget2;
    // the swap rt associated with render targets for offscreen rendering
    // this uses index inherited from Descriptor
    ID3D12Resource* swapRenderTarget;
    // keep track of currently used RT and last used RT 
    unsigned activeRTIndex;
    unsigned activeSRVIndex;
    ID3D12Resource* activeRT;
    // last used
    unsigned lastRTIndex;
    unsigned lastSRVIndex;
    ID3D12Resource* lastRT;

    std::function<void(unsigned, unsigned, unsigned, unsigned)> freeRT;

    // UGLY!!!
    void SwapActive()
    {
      lastRT = activeRT;
      lastRTIndex = activeRTIndex;
      lastSRVIndex = activeSRVIndex;

      if (activeRT == renderTarget2.Get())
      {
        activeRT = renderTarget1.Get();
        activeRTIndex = renderTargetIndex1;
        activeSRVIndex = shaderResourceIndex1;
        return;
      }

      activeRT = renderTarget2.Get();
      activeRTIndex = renderTargetIndex2;
      activeSRVIndex = shaderResourceIndex2;
    }

    ~RenderTargetDescriptor()
    {
      renderTarget1.Reset();
      renderTarget2.Reset();

      if (freeRT)
        freeRT(
          renderTargetIndex1, shaderResourceIndex1, renderTargetIndex2, shaderResourceIndex2);
    }
  };

  class ResourceManager
  {
    // fixed size of 64k, (max supported by DX12)
    const unsigned RESOURCE_HEAP_SIZE = 65535;
    const unsigned DSV_HEAP_SIZE = 1;
    const unsigned RTV_HEAP_SIZE = 2 + (2 * 2); // double buffering (for offscreen rendering each framebuffer need 2 RenderTargets)
    const unsigned SAMPLER_HEAP_SIZE = 2048; // overkill reduce this later
    // resources are ordered in regions, assuming that I will be loading 1500 meshes at once
    const Range CB_RANGE = { 0, 7499 }; // for each mesh 5 CBVs
    const Range TEX_RANGE = { 7500, 14999 }; // since PBR is planned, normally we need 5 textures per mesh
    const Range MIPS_RANGE = { 15000, 44999 }; // for each texture we have 4 mips
    // has its own heap
    const Range RT_RANGE = { 0, RTV_HEAP_SIZE - 1 }; // for each texture we have 4 mips
    const Range DS_RANGE = { 0, DSV_HEAP_SIZE - 1 };
    const Range SAMPLER_RANGE = { 0, SAMPLER_HEAP_SIZE - 1 };

  public:
    static ResourceManager& Instance()
    {
      static ResourceManager instance;
      return instance;
    }
    ~ResourceManager();

    // Getters
    ID3D12DescriptorHeap* GetResourcesHeap() { return m_resourcesHeap.Get(); }
    ID3D12DescriptorHeap* GetRTVHeap() { return m_rtvHeap.Get(); }
    ID3D12DescriptorHeap* GetDSVHeap() { return m_dsvHeap.Get(); }
    ID3D12DescriptorHeap* GetSamplerHeap() { return m_samplerHeap.Get(); }

    // for textures, CBV ...etc
    D3D12_GPU_DESCRIPTOR_HANDLE GetResourceGpuHandle(unsigned index);
    D3D12_CPU_DESCRIPTOR_HANDLE GetResourceCpuHandle(unsigned index);
    D3D12_GPU_DESCRIPTOR_HANDLE GetRTVGpuHandle(unsigned index);
    D3D12_CPU_DESCRIPTOR_HANDLE GetRTVCpuHandle(unsigned index);
    D3D12_GPU_DESCRIPTOR_HANDLE GetDSVGpuHandle(unsigned index);
    D3D12_CPU_DESCRIPTOR_HANDLE GetDSVCpuHandle(unsigned index);
    D3D12_GPU_DESCRIPTOR_HANDLE GetSamplerGpuHandle(unsigned index);
    D3D12_CPU_DESCRIPTOR_HANDLE GetSamplerCpuHandle(unsigned index);

    std::unique_ptr<ResourceDescriptor> CreateConstantBufferResource(size_t size, D3D12_HEAP_TYPE type);
    // right now I believe the upload buffer is not freed after the texture has been uploaded to the GPU
    // TODO: remove free the upload resource
    std::shared_ptr<TextureDescriptor> CreateTextureResource(D3D12_RESOURCE_DESC& desc, bool isCubeMap, bool generateMips);
    std::unique_ptr<ResourceDescriptor> CreateDepthResource(D3D12_RESOURCE_DESC& desc, D3D12_CLEAR_VALUE& clearValue);
    // view and not resource because the swap chain is the one that owns RT resources
    // index to be removed when the views are properly tracked
    std::shared_ptr<RenderTargetDescriptor> CreateRenderTargetResource(ID3D12Resource* swapRenderTarget);
    // also sampler will have no resource
    std::unique_ptr<Descriptor> CreateSampler(D3D12_SAMPLER_DESC& desc);

  private:
    ComPtr<ID3D12DescriptorHeap> m_resourcesHeap;
    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
    ComPtr<ID3D12DescriptorHeap> m_samplerHeap;
    // track free heap places
    std::vector<unsigned> m_nextFreeTex;
    std::vector<unsigned> m_nextFreeMip;
    std::vector<unsigned> m_nextFreeCB;
    std::vector<unsigned> m_nextFreeRT;
    std::vector<unsigned> m_nextFreeDS;
    std::vector<unsigned> m_nextFreeSampler;
    // mutex
    std::mutex m_mutexTex;
    std::mutex m_mutexMip;
    std::mutex m_mutexCB;
    std::mutex m_mutexRT;
    std::mutex m_mutexDS;
    std::mutex m_mutexSampler;

  private:
    ResourceManager();
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

  };
}
