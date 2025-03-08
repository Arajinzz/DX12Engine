#pragma once

namespace Core
{
  class DX12Context
  {
  public:
    DX12Context();
    ~DX12Context();

  private:
    DX12Context(const DX12Context&) = delete;
    DX12Context& operator=(const DX12Context&) = delete;

  };
}

