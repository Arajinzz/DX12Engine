#pragma once

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace Core
{
  // This will provide functions to manage DX12,
// like creating buffers, heaps, root signatures ...etc.
// It will be a singleton.
  class DX12Interface
  {
  public:
    static DX12Interface& Get()
    {
      static DX12Interface instance;
      return instance;
    }

    inline ID3D12Device* GetDevice()
    {
      return m_device.Get();
    }

    ~DX12Interface();

  protected:
    void Initialize();
  
  public:
    ComPtr<ID3D12Resource> CreateConstantBuffer(size_t size, D3D12_HEAP_TYPE type);
    // only direct CommandQueue
    ComPtr<ID3D12CommandQueue> CreateCommandQueue();
    ComPtr<ID3D12Fence> CreateFence();
    ComPtr<ID3D12CommandAllocator> CreateCommandAllocator();
    ComPtr<ID3D12GraphicsCommandList> CreateCommandList(std::vector<ComPtr<ID3D12CommandAllocator>> allocators);
    ComPtr<ID3D12DescriptorHeap> CreateHeapDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type, unsigned size);
    ComPtr<IDXGISwapChain1> CreateSwapChainForHwnd(DXGI_SWAP_CHAIN_DESC1& desc, HWND windowHandle, ID3D12CommandQueue* commandQueue);

    void CreateRenderTargetView(ID3D12Resource* resource, ID3D12DescriptorHeap* heap, unsigned offset);
    void CreateDepthStencilView(ID3D12Resource* resource, ID3D12DescriptorHeap* heap, unsigned offset);
    void CreateShaderResourceView(ID3D12Resource* resource, ID3D12DescriptorHeap* heap, unsigned offset, bool isCubeMap = false);
    void CreateUnorderedAccessView(ID3D12Resource* resource, ID3D12DescriptorHeap* heap, unsigned offset, unsigned currentMipLevel = 0);
    void CreateConstantBufferView(ID3D12Resource* resource, ID3D12DescriptorHeap* heap, unsigned offset);
    void CreateSampler(D3D12_SAMPLER_DESC* desc, ID3D12DescriptorHeap* heap, unsigned offset);

    void MakeWindowAssociation(HWND windowHandle, unsigned flags);

    unsigned GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE type);

  private:
    void GetHardwareAdapter(
      _In_ IDXGIFactory1* pFactory,
      _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter,
      bool requestHighPerformanceAdapter = false);

  private:
    ComPtr<ID3D12Device> m_device;
    ComPtr<IDXGIFactory4> m_factory;

  private:
    DX12Interface();
    DX12Interface(const DX12Interface&) = delete;
    DX12Interface& operator=(const DX12Interface&) = delete;
  };
}
