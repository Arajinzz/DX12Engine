#pragma once

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

  struct ResourceDescriptor
  {
    ComPtr<ID3D12Resource> resource;
    unsigned index;
    std::function<void(unsigned)> freeResource;

    virtual ~ResourceDescriptor()
    {
      resource.Reset();
      if (freeResource)
        freeResource(index);
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

  class ResourceManager
  {
    // fixed size of 64k, (max supported by DX12)
    const unsigned RESOURCE_HEAP_SIZE = 65535;
    const unsigned DSV_HEAP_SIZE = 1;
    const unsigned RTV_HEAP_SIZE = 2; // double buffering
    // resources are ordered in regions, assuming that I will be loading 1500 meshes at once
    const Range CB_RANGE = { 0, 7499 }; // for each mesh 5 CBVs
    const Range TEX_RANGE = { 7500, 14999 }; // since PBR is planned, normally we need 5 textures per mesh
    const Range MIPS_RANGE = { 15000, 44999 }; // for each texture we have 4 mips

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

    // for textures, CBV ...etc
    D3D12_GPU_DESCRIPTOR_HANDLE GetResourceGpuHandle(unsigned index);
    D3D12_CPU_DESCRIPTOR_HANDLE GetResourceCpuHandle(unsigned index);
    D3D12_GPU_DESCRIPTOR_HANDLE GetRTVGpuHandle(unsigned index);
    D3D12_CPU_DESCRIPTOR_HANDLE GetRTVCpuHandle(unsigned index);
    D3D12_GPU_DESCRIPTOR_HANDLE GetDSVGpuHandle(unsigned index);
    D3D12_CPU_DESCRIPTOR_HANDLE GetDSVCpuHandle(unsigned index);

    std::unique_ptr<ResourceDescriptor> CreateConstantBufferResource(size_t size, D3D12_HEAP_TYPE type);
    std::unique_ptr<TextureDescriptor> CreateTextureResource(D3D12_RESOURCE_DESC& desc, bool isCubeMap, bool generateMips);
    std::unique_ptr<ResourceDescriptor> CreateDepthResource(D3D12_RESOURCE_DESC& desc, D3D12_CLEAR_VALUE& clearValue);
    // view and not resource because the swap chain is the one that owns RT resources
    // index to be removed when the views are properly tracked
    std::unique_ptr<ResourceDescriptor> CreateRenderTargetView(ID3D12Resource* renderTarget, unsigned index);

  private:
    ComPtr<ID3D12DescriptorHeap> m_resourcesHeap;
    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
    // track free heap places
    std::vector<unsigned> m_nextFreeTex;
    std::vector<unsigned> m_nextFreeMip;
    std::vector<unsigned> m_nextFreeCB;

  private:
    ResourceManager();
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

  };
}
