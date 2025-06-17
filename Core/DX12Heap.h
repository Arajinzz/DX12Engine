#pragma once

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace Core
{
  class DX12SwapChain;

  enum ResourceType
  {
    TEXTURE,
    CONSTANTBUFFER,
    DEPTH,
    RENDERTARGET,
    CUBEMAP,
    UAV
  };

  class DX12Heap
  {
  public:
    DX12Heap(D3D12_DESCRIPTOR_HEAP_TYPE type);
    ~DX12Heap();

    void CreateHeap();
    void ResetResources();
    void ResetHeap();

    void AddResource(ComPtr<ID3D12Resource> resource, ResourceType type);
    void CreateViews();

    CD3DX12_CPU_DESCRIPTOR_HANDLE GetOffsetHandle(unsigned int Offset);
    CD3DX12_GPU_DESCRIPTOR_HANDLE GetOffsetGPUHandle(unsigned int Offset);
    ID3D12Resource* GetResource(unsigned int index) { return m_resources[index].Get(); }
    ID3D12DescriptorHeap* Get() { return m_heap.Get(); }

  private:
    DX12Heap(const DX12Heap&) = delete;
    DX12Heap& operator=(const DX12Heap&) = delete;

  private:
    ComPtr<ID3D12DescriptorHeap> m_heap;
    std::vector<ComPtr<ID3D12Resource>> m_resources;
    std::vector<ResourceType> m_resourceTypes;
    unsigned int m_descriptorSize;
    D3D12_DESCRIPTOR_HEAP_TYPE m_type;
    CD3DX12_CPU_DESCRIPTOR_HANDLE m_handle;
  };
}
