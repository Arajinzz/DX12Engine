#pragma once

namespace Core
{
  class SkyboxPass
  {
  public:
    SkyboxPass();
    ~SkyboxPass();

  private:
    SkyboxPass(const SkyboxPass&) = delete;
    SkyboxPass& operator=(const SkyboxPass&) = delete;

  };
}
