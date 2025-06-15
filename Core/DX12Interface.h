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

    inline IDXGIFactory4* GetFactory()
    {
      return m_factory.Get();
    }

    ~DX12Interface();

  public:
    ComPtr<ID3D12Resource> CreateBuffer(size_t size, D3D12_HEAP_TYPE type);

  protected:
    void Initialize();

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
