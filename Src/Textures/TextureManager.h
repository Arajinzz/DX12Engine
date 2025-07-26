#pragma once

#include "Core/ResourceManager.h"
#include "Core/DX12Texture.h"

#include <unordered_map>

using namespace DirectX;
using Microsoft::WRL::ComPtr;

// A very basic texture manager
namespace Core
{
  class TextureManager
  {
  public:
    static TextureManager& Instance()
    {
      static TextureManager instance;
      return instance;
    }
    ~TextureManager();

    std::shared_ptr<DX12Texture> CreateOrGetTexture(const std::vector<std::string>& paths);

  private:
    // when texture is deleted it will not be removed from this map
    // but the resource will be freed
    std::unordered_map<size_t, std::weak_ptr<DX12Texture>> m_textures;

  private:
    TextureManager();
    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;
  };

}
