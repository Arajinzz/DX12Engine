#pragma once

#include "Core/DXApplicationHelper.h"

#include <unordered_map>
#include <filesystem>

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace Core
{
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

  class ShaderManager
  {
  public:
    static ShaderManager& Instance()
    {
      static ShaderManager instance;
      return instance;
    }
    ~ShaderManager();

    const std::shared_ptr<ShaderBlob> GetShader(const std::string& shaderName) { return m_shaderMap[shaderName]; }

  private:
    // function that reads and compiles all available shaders
    void RegisterShaders();

  private:
    // shader map
    std::unordered_map<std::string, std::shared_ptr<ShaderBlob>> m_shaderMap;

  private:
    ShaderManager();
    ShaderManager(const ShaderManager&) = delete;
    ShaderManager& operator=(const ShaderManager&) = delete;

  };
}
