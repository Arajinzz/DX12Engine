#include "stdafx.h"
#include "DX12Interface.h"
#include "Core/DXApplicationHelper.h"

namespace Graphics
{
  DX12Interface::DX12Interface()
    : m_device(nullptr)
    , m_factory(nullptr)
  {
    // initialize the hardware adapter
    Initialize();
  }


  DX12Interface::~DX12Interface()
  {
    m_device.Reset();
    m_factory.Reset();
  }

  void DX12Interface::Initialize()
  {
    unsigned int dxgiFactoryFlags = 0;

#if defined(_DEBUG)
    // Enable the debug layer (requires the Graphics Tools "optional feature").
    // NOTE: Enabling the debug layer after device creation will invalidate the active device.
    {
      ComPtr<ID3D12Debug> debugController;
      if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
      {
        debugController->EnableDebugLayer();

        // Enable additional debug layers.
        dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
      }
    }
#endif

    // Create Factory
    ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_factory)));

    // Create device
    ComPtr<IDXGIAdapter1> hardwareAdapter;
    GetHardwareAdapter(m_factory.Get(), &hardwareAdapter, true);

    ThrowIfFailed(D3D12CreateDevice(
      hardwareAdapter.Get(),
      D3D_FEATURE_LEVEL_11_0,
      IID_PPV_ARGS(&m_device)
    ));

#if defined(_DEBUG)
    ComPtr<ID3D12InfoQueue> pInfoQueue;
    if (SUCCEEDED(m_device.As(&pInfoQueue)))
    {
      pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
      pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
      pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

      // Suppress whole categories of messages
      //D3D12_MESSAGE_CATEGORY Categories[] = {};

      // Suppress messages based on their severity level
      D3D12_MESSAGE_SEVERITY Severities[] =
      {
          D3D12_MESSAGE_SEVERITY_INFO
      };

      // Suppress individual messages by their ID
      D3D12_MESSAGE_ID DenyIds[] = {
          D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,   // I'm really not sure how to avoid this message.
          D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,                         // This warning occurs when using capture frame while graphics debugging.
          D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,                       // This warning occurs when using capture frame while graphics debugging.
      };

      D3D12_INFO_QUEUE_FILTER NewFilter = {};
      //NewFilter.DenyList.NumCategories = _countof(Categories);
      //NewFilter.DenyList.pCategoryList = Categories;
      NewFilter.DenyList.NumSeverities = _countof(Severities);
      NewFilter.DenyList.pSeverityList = Severities;
      NewFilter.DenyList.NumIDs = _countof(DenyIds);
      NewFilter.DenyList.pIDList = DenyIds;

      ThrowIfFailed(pInfoQueue->PushStorageFilter(&NewFilter));
    }
#endif
  }

  // Helper function for acquiring the first available hardware adapter that supports Direct3D 12.
  // If no such adapter can be found, *ppAdapter will be set to nullptr.
  _Use_decl_annotations_
    void DX12Interface::GetHardwareAdapter(IDXGIFactory1* pFactory, IDXGIAdapter1** ppAdapter, bool requestHighPerformanceAdapter)
  {
    *ppAdapter = nullptr;

    ComPtr<IDXGIAdapter1> adapter;

    ComPtr<IDXGIFactory6> factory6;
    if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
    {
      for (
        UINT adapterIndex = 0;
        SUCCEEDED(factory6->EnumAdapterByGpuPreference(
          adapterIndex,
          requestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
          IID_PPV_ARGS(&adapter)));
          ++adapterIndex)
      {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
          // Don't select the Basic Render Driver adapter.
          // If you want a software adapter, pass in "/warp" on the command line.
          continue;
        }

        // Check to see whether the adapter supports Direct3D 12, but don't create the
        // actual device yet.
        if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
        {
          break;
        }
      }
    }

    if (adapter.Get() == nullptr)
    {
      for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
      {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
          // Don't select the Basic Render Driver adapter.
          // If you want a software adapter, pass in "/warp" on the command line.
          continue;
        }

        // Check to see whether the adapter supports Direct3D 12, but don't create the
        // actual device yet.
        if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
        {
          break;
        }
      }
    }

    *ppAdapter = adapter.Detach();
  }

  ComPtr<ID3D12Resource> DX12Interface::CreateConstantBuffer(size_t size, D3D12_HEAP_TYPE type)
  {
    auto heapProperties = CD3DX12_HEAP_PROPERTIES(type);
    auto resourceDescription = CD3DX12_RESOURCE_DESC::Buffer(size);

    ComPtr<ID3D12Resource> buffer;

    ThrowIfFailed(m_device->CreateCommittedResource(
      &heapProperties,
      D3D12_HEAP_FLAG_NONE,
      &resourceDescription,
      D3D12_RESOURCE_STATE_GENERIC_READ,
      nullptr,
      IID_PPV_ARGS(&buffer)));

    return buffer;
  }

  ComPtr<ID3D12CommandQueue> DX12Interface::CreateCommandQueue()
  {
    // Describe and create the command queue.
    ComPtr<ID3D12CommandQueue> commandQueue;
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    ThrowIfFailed(DX12Interface::Get().GetDevice()->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)));
    return commandQueue;
  }

  ComPtr<ID3D12Fence> DX12Interface::CreateFence()
  {
    ComPtr<ID3D12Fence> fence;
    // initial value of 0
    ThrowIfFailed(DX12Interface::Get().GetDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
    return fence;
  }

  ComPtr<ID3D12CommandAllocator> DX12Interface::CreateCommandAllocator()
  { // supports only direct for now
    ComPtr<ID3D12CommandAllocator> commandAllocator;
    ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)));
    return commandAllocator;
  }

  ComPtr<ID3D12GraphicsCommandList> DX12Interface::CreateCommandList(std::vector<ComPtr<ID3D12CommandAllocator>> allocators)
  { // supports only direct for now
    ComPtr<ID3D12GraphicsCommandList> commandList;
    for (auto allocator : allocators)
      ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator.Get(), nullptr, IID_PPV_ARGS(&commandList)));
    return commandList;
  }

  ComPtr<ID3D12DescriptorHeap> DX12Interface::CreateHeapDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type, unsigned size)
  {
    ComPtr<ID3D12DescriptorHeap> heap;
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.NumDescriptors = size;
    heapDesc.Type = type;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    if (type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
      heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(DX12Interface::Get().GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&heap)));

    return heap;
  }

  void DX12Interface::CreateRenderTargetView(ID3D12Resource* resource, ID3D12DescriptorHeap* heap, unsigned offset)
  {
    auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
      heap->GetCPUDescriptorHandleForHeapStart(), offset, m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
    m_device->CreateRenderTargetView(resource, nullptr, handle);
  }

  void DX12Interface::CreateDepthStencilView(ID3D12Resource* resource, ID3D12DescriptorHeap* heap, unsigned offset)
  {
    auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
      heap->GetCPUDescriptorHandleForHeapStart(), offset, m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV));

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    m_device->CreateDepthStencilView(resource, &dsvDesc, handle);
  }

  void DX12Interface::CreateShaderResourceView(ID3D12Resource* resource, ID3D12DescriptorHeap* heap, unsigned offset, bool isCubeMap)
  {
    auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
      heap->GetCPUDescriptorHandleForHeapStart(), offset, m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = isCubeMap ? D3D12_SRV_DIMENSION_TEXTURECUBE : D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = isCubeMap ? 1 : resource->GetDesc().MipLevels; // cubemap does not support mipmaps
    m_device->CreateShaderResourceView(resource, &srvDesc, handle);
  }

  void DX12Interface::CreateUnorderedAccessView(ID3D12Resource* resource, ID3D12DescriptorHeap* heap, unsigned offset, unsigned currentMipLevel)
  {
    auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
      heap->GetCPUDescriptorHandleForHeapStart(), offset, m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
    uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    uavDesc.Texture2D.MipSlice = currentMipLevel;
    m_device->CreateUnorderedAccessView(resource, nullptr, &uavDesc, handle);
  }

  void DX12Interface::CreateConstantBufferView(ID3D12Resource* resource, ID3D12DescriptorHeap* heap, unsigned offset)
  {
    auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
      heap->GetCPUDescriptorHandleForHeapStart(), offset, m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

    // Describe and create a constant buffer view
    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
    cbvDesc.BufferLocation = resource->GetGPUVirtualAddress();
    cbvDesc.SizeInBytes = static_cast<unsigned>(resource->GetDesc().Width);
    m_device->CreateConstantBufferView(&cbvDesc, handle);
  }

  void DX12Interface::CreateSampler(D3D12_SAMPLER_DESC* desc, ID3D12DescriptorHeap* heap, unsigned offset)
  {
    auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
      heap->GetCPUDescriptorHandleForHeapStart(), offset, m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER));
    m_device->CreateSampler(desc, handle);
  }

  void DX12Interface::MakeWindowAssociation(HWND windowHandle, unsigned flags)
  {
    ThrowIfFailed(m_factory->MakeWindowAssociation(windowHandle, flags));
  }

  unsigned DX12Interface::GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE type)
  {
    return m_device->GetDescriptorHandleIncrementSize(type);
  }

  ComPtr<IDXGISwapChain1> DX12Interface::CreateSwapChainForHwnd(DXGI_SWAP_CHAIN_DESC1& desc, HWND windowHandle, ID3D12CommandQueue* commandQueue)
  {
    ComPtr<IDXGISwapChain1> swapChain;
    ThrowIfFailed(m_factory->CreateSwapChainForHwnd(
      commandQueue, // Swap chain needs the queue so that it can force a flush on it.
      windowHandle,
      &desc,
      nullptr,
      nullptr,
      &swapChain
    ));

    return swapChain;
  }

}
