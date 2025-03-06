#pragma once

#include "Core/DX12CommandList.h"
#include "Core/DX12Texture.h"
#include "Core/DX12ConstantBuffer.h"

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
    
    void Init(ID3D12GraphicsCommandList* commandList);
    void Update();
    DX12Heap* GetHeap() { return m_CbvSrvHeap.get(); }
    
    ~DX12FrameResource();

  private:
    struct SceneConstantBuffer
    {
      XMMATRIX model;
      XMMATRIX view;
      XMMATRIX projection;
      float padding[16]; // Padding so the constant buffer is 256-byte aligned.
    };

    std::unique_ptr<DX12CommandList> m_commandList;
    std::unique_ptr<DX12Heap> m_CbvSrvHeap;
    std::unique_ptr<DX12Texture> m_texture;
    std::unique_ptr<DX12ConstantBuffer> m_constantBuffer;

    // data
    SceneConstantBuffer m_constantBufferData;
    UINT8* m_pCbvDataBegin;

  private:
    DX12FrameResource();
    DX12FrameResource(const DX12FrameResource&) = delete;
    DX12FrameResource& operator=(const DX12FrameResource&) = delete;
  };
}

