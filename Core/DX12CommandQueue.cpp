#include "stdafx.h"
#include "DX12CommandQueue.h"

#include "Core/DXApplicationHelper.h"
#include "Core/DX12Device.h"

namespace Core
{
  DX12CommandQueue::DX12CommandQueue()
  {
    // Describe and create the command queue.
    {
      D3D12_COMMAND_QUEUE_DESC queueDesc = {};
      queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
      queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
      ThrowIfFailed(Device()->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));
    }
  }

  DX12CommandQueue::~DX12CommandQueue()
  {
    m_commandQueue.Reset();
  }
}