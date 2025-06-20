#pragma once

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace Core
{
  struct ResourceDescriptor
  {
    ComPtr<ID3D12Resource> resource;
    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
  };

  class ResourceManager
  {
  public:
    static ResourceManager& Instance()
    {
      static ResourceManager instance;
      return instance;
    }
    ~ResourceManager();

    // Getters
    ID3D12DescriptorHeap* GetTexHeap() { return m_texHeap.Get(); }
    ID3D12DescriptorHeap* GetMipsHeap() { return m_mipsHeap.Get(); }

    ResourceDescriptor CreateTextureResource(
      D3D12_RESOURCE_DESC& desc, CD3DX12_HEAP_PROPERTIES& props, D3D12_RESOURCE_STATES state, bool createView);
    ResourceDescriptor CreateMipsForTexture(ResourceDescriptor texture);

  private:
    std::vector<ResourceDescriptor> m_texResources;
    ComPtr<ID3D12DescriptorHeap> m_texHeap;

    std::vector<ResourceDescriptor> m_mipsResources;
    ComPtr<ID3D12DescriptorHeap> m_mipsHeap;

  private:
    ResourceManager();
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

  };
}
