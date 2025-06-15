#include "stdafx.h"
#include "DX12Interface.h"
#include "Core/DXApplicationHelper.h"

namespace Core
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

  ComPtr<ID3D12Resource> DX12Interface::CreateBuffer(size_t size, D3D12_HEAP_TYPE type)
  {
    auto heapProperties = CD3DX12_HEAP_PROPERTIES(type);
    auto resourceDescription = CD3DX12_RESOURCE_DESC::Buffer(size);

    ComPtr<ID3D12Resource> buffer;

    ThrowIfFailed(DX12Interface::Get().GetDevice()->CreateCommittedResource(
      &heapProperties,
      D3D12_HEAP_FLAG_NONE,
      &resourceDescription,
      D3D12_RESOURCE_STATE_GENERIC_READ,
      nullptr,
      IID_PPV_ARGS(&buffer)));

    return buffer;
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
}
