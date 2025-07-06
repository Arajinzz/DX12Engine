#pragma once

#include "Core/DXApplicationHelper.h"

#include <unordered_map>
#include <filesystem>

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace Core
{
  enum
  {
    GenerateMipsCB,
    SrcMip,
    OutMip,
    NumRootParameters
  };

  struct SGenerateMipsCB
  {
    uint32_t SrcMipLevel;           // Texture level of source mip
    uint32_t NumMipLevels;          // Number of OutMips to write: [1-4]
    uint32_t SrcDimension;          // Width and height of the source texture are even or odd.
    uint32_t IsSRGB;                // Must apply gamma correction to sRGB textures.
    DirectX::XMFLOAT2 TexelSize;    // 1.0 / OutMip1.Dimensions
  };

  struct ShaderBlob
  {
    ComPtr<ID3DBlob> vertexShader;
    ComPtr<ID3DBlob> pixelShader;
    ComPtr<ID3DBlob> computeShader;
    ComPtr<ID3DBlob> errorBlob;

    ShaderBlob(const std::string& path, bool isCompute)
    {
      // Enable better shader debugging with the graphics debugging tools.
      UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
      auto fullPath = std::filesystem::current_path().string() + "/" + path;

      // Convert char* to std::wstring
      int wchars_num = MultiByteToWideChar(CP_UTF8, 0, fullPath.c_str(), -1, nullptr, 0);
      std::wstring wFullPath(wchars_num, 0);
      MultiByteToWideChar(CP_UTF8, 0, fullPath.c_str(), -1, &wFullPath[0], wchars_num);

      if (isCompute)
      {
        ThrowIfFailed(D3DCompileFromFile(wFullPath.c_str(), nullptr, nullptr, "main", "cs_5_1", compileFlags, 0, &computeShader, &errorBlob));
      } else
      {
        ThrowIfFailed(D3DCompileFromFile(wFullPath.c_str(), nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, &errorBlob));
        ThrowIfFailed(D3DCompileFromFile(wFullPath.c_str(), nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, &errorBlob));
      }
    }

    ~ShaderBlob()
    {
      vertexShader.Reset();
      pixelShader.Reset();
      errorBlob.Reset();
    }
  };

  struct PSO
  {
    ComPtr<ID3D12RootSignature> m_rootSignature;
    ComPtr<ID3D12PipelineState> m_pipelineState;

    ~PSO()
    {
      m_pipelineState.Reset();
      m_rootSignature.Reset();
    }
  };

  class ShaderManager
  {
  public:
    static ShaderManager& Instance()
    {
      static ShaderManager instance;
      return instance;
    }
    ~ShaderManager();

    PSO GetShader(const std::string& shaderName) { return m_PSOs[shaderName]; }

  private:
    // function that reads and compiles all available shaders
    void RegisterShaders();

  private:
    // go simple for now, use shader file name as a key
    // refactor this as needed
    std::unordered_map<std::string, PSO> m_PSOs;
    // shader map
    std::unordered_map<std::string, std::shared_ptr<ShaderBlob>> m_shaderMap;

  private:
    ShaderManager();
    ShaderManager(const ShaderManager&) = delete;
    ShaderManager& operator=(const ShaderManager&) = delete;

  };
}
