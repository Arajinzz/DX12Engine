#pragma once

#include "Core/DXApplicationHelper.h"

#include <unordered_map>

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace Core
{
  struct ShaderBlob
  {
    std::wstring filename;
    ComPtr<ID3DBlob> vertexShader;
    ComPtr<ID3DBlob> pixelShader;
    ComPtr<ID3DBlob> errorBlob;

    ShaderBlob(const std::wstring& path)
    {
      // Enable better shader debugging with the graphics debugging tools.
      UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;

      auto GetAssetFullPath = [&](std::wstring assetName) {
        WCHAR assetsPath[512];
        GetAssetsPath(assetsPath, _countof(assetsPath));
        return std::wstring(assetsPath) + assetName;
        };

      ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(path).c_str(), nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, &errorBlob));
      ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(path).c_str(), nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));

      // set the file name
      filename = path.substr(path.find_last_of(L"/\\") + 1);
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

    PSO GetShader(const std::wstring& shaderName) { return m_PSOs[shaderName]; }

  private:
    // function that reads and compiles all available shaders
    void RegisterShaders();

  private:
    // go simple for now, use shader file name as a key
    // refactor this as needed
    std::unordered_map<std::wstring, PSO> m_PSOs;

  private:
    ShaderManager();
    ShaderManager(const ShaderManager&) = delete;
    ShaderManager& operator=(const ShaderManager&) = delete;

  };
}
