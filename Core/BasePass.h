#pragma once

namespace Core
{
  class BasePass
  {
  public:
    BasePass();
    ~BasePass();

  private:
    BasePass(const BasePass&) = delete;
    BasePass& operator=(const BasePass&) = delete;
  };
}
