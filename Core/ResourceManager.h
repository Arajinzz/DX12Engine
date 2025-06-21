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
    // fixed size of 8k
    const unsigned HEAP_SIZE = 8000;
    const Range TEX_RANGE = { 0, 999 };
    const Range MIPS_RANGE = { 1000, 4999 };
    const Range CB_RANGE = { 5000, 7999 };

  public:
    static ResourceManager& Instance()
    {
      static ResourceManager instance;
      return instance;
    }
    ~ResourceManager();

    // Getters
    ID3D12DescriptorHeap* GetHeap() { return m_heap.Get(); }
    D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle(unsigned index);
    D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle(unsigned index);

    TextureDescriptor CreateTextureResource(D3D12_RESOURCE_DESC& desc, bool isCubeMap, bool generateMips);

  private:
    std::vector<ResourceDescriptor> m_resources;
    ComPtr<ID3D12DescriptorHeap> m_heap;
    // track free heap places
    std::vector<unsigned> m_nextFreeTex;
    std::vector<unsigned> m_nextFreeMip;

  private:
    ResourceManager();
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

  };
}
