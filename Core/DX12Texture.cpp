#include "stdafx.h"
#include "DX12Texture.h"

namespace
{
  std::vector<UINT8> GenerateTextureData(unsigned texW, unsigned texH, unsigned pixelSize)
  {
    const UINT rowPitch = texW * pixelSize;
    const UINT cellPitch = rowPitch >> 3; // The width of a cell in the checkboard texture.
    const UINT cellHeight = texW >> 3;    // The height of a cell in the checkerboard texture.
    const UINT textureSize = rowPitch * texH;

    std::vector<UINT8> data(textureSize);
    UINT8* pData = &data[0];

    for (UINT n = 0; n < textureSize; n += pixelSize)
    {
      UINT x = n % rowPitch;
      UINT y = n / rowPitch;
      UINT i = x / cellPitch;
      UINT j = y / cellHeight;

      if (i % 2 == j % 2)
      {
        pData[n] = 0x00;        // R
        pData[n + 1] = 0x00;    // G
        pData[n + 2] = 0x00;    // B
        pData[n + 3] = 0xff;    // A
      } else
      {
        pData[n] = 0xff;        // R
        pData[n + 1] = 0xff;    // G
        pData[n + 2] = 0xff;    // B
        pData[n + 3] = 0xff;    // A
      }
    }

    return data;
  }
}

namespace Core
{
  DX12Texture::DX12Texture()
  {
  }

  DX12Texture::~DX12Texture()
  {
  }
}
