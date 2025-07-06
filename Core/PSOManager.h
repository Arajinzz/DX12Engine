#pragma once

namespace Core
{
  class PSOManager
  {
  public:
    static PSOManager& Instance()
    {
      static PSOManager instance;
      return instance;
    }
    ~PSOManager();

  private:
    PSOManager();
    PSOManager(const PSOManager&) = delete;
    PSOManager& operator=(const PSOManager&) = delete;

  };

}
