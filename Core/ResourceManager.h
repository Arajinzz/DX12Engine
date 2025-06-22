#pragma once

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
  };

  struct TextureDescriptor : public ResourceDescriptor
  {
    ComPtr<ID3D12Resource> upload;
    unsigned mipIndex;
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
    D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle(unsigned index);
    D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle(unsigned index);

    ResourceDescriptor CreateConstantBufferResource(size_t size, D3D12_HEAP_TYPE type);
    TextureDescriptor CreateTextureResource(D3D12_RESOURCE_DESC& desc, bool isCubeMap, bool generateMips);

  private:
    ComPtr<ID3D12DescriptorHeap> m_resourcesHeap;
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
