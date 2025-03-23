#pragma once

namespace Core
{
  class DX12MipsGenerator
  {
  public:
    DX12MipsGenerator();
    ~DX12MipsGenerator();

  private:
    DX12MipsGenerator(const DX12MipsGenerator&) = delete;
    DX12MipsGenerator& operator=(const DX12MipsGenerator&) = delete;
  };
}

