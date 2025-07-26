#include "stdafx.h"
#include "TextureManager.h"

namespace Textures
{
  TextureManager::TextureManager()
    : m_textures()
  {
  }

  TextureManager::~TextureManager()
  {
    m_textures.clear();
  }

  std::shared_ptr<DX12Texture> TextureManager::CreateOrGetTexture(const std::vector<std::string>& paths)
  {
    std::hash<std::string> hashFct;
    size_t hash = 0;
    // generate the hash
    for (auto path : paths)
      hash += hashFct(path);

    // check if it exists
    if (m_textures.find(hash) != m_textures.end())
      if (std::shared_ptr<DX12Texture> shared = m_textures[hash].lock())
        return shared;

    // does not exist
    // create it, and load it
    auto texture = std::make_shared<DX12Texture>(paths, paths.size() == 1 ? 4 : 1); // no mips for cubemap
   
    // store in map, since it will be casted to weak ptr the ref count will not be increased
    // because when the texture is not used by anyone it is supposed to be freed
    m_textures[hash] = texture;

    return texture;
  }
}
