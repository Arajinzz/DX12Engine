#pragma once

using namespace DirectX;
using Microsoft::WRL::ComPtr;

#define CreateDevice() DX12Device::Instance()
#define Device() DX12Device::Instance().GetDevice()
#define Factory() DX12Device::Instance().GetFactory()

// Can be better
namespace Core
{
  // for now going for a singleton
  class DX12Device
  {
  public:
    static DX12Device& Instance()
    {
      static DX12Device instance;
      return instance;
    }

    ID3D12Device* GetDevice()
    {
      return m_device.Get();
    }

    IDXGIFactory4* GetFactory()
    {
      return m_factory.Get();
    }

    ~DX12Device();

    DX12Device(const DX12Device&) = delete;
    DX12Device& operator= (const DX12Device&) = delete;

  private:
    DX12Device();

    void GetHardwareAdapter(
      _In_ IDXGIFactory1* pFactory,
      _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter,
      bool requestHighPerformanceAdapter = false);

  private:
    ComPtr<ID3D12Device> m_device;
    ComPtr<IDXGIFactory4> m_factory;

  };
}

