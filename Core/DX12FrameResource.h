#pragma once

#include "Core/DX12CommandList.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

#define CreateFrameResource() DX12FrameResource::Instance()
#define FrameResource() DX12FrameResource::Instance()

namespace Core
{
  class DX12FrameResource
  {
  public:
    static DX12FrameResource& Instance()
    {
      static DX12FrameResource instance;
      return instance;
    }
    ~DX12FrameResource();

  private:
    std::unique_ptr<DX12CommandList> m_commandList;
    std::unique_ptr<DX12Heap> m_CbvSrvHeap;

  private:
    DX12FrameResource();
    DX12FrameResource(const DX12FrameResource&) = delete;
    DX12FrameResource& operator=(const DX12FrameResource&) = delete;
  };
}

