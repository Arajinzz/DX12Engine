#include "stdafx.h"
#include "DX12Shader.h"

#include "Core/DX12Device.h"
#include "Core/DXApplicationHelper.h"

namespace Core
{
  DX12Shader::DX12Shader(const wchar_t* path)
    : m_rootParameters()
  {
    ComPtr<ID3DBlob> errorBlob;
    // Enable better shader debugging with the graphics debugging tools.
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;

    auto GetAssetFullPath = [&](std::wstring assetName) {
      WCHAR assetsPath[512];
      GetAssetsPath(assetsPath, _countof(assetsPath));
      return std::wstring(assetsPath) + assetName;
      };

    ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(path).c_str(), nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &m_vertexShader, &errorBlob));
    ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(path).c_str(), nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &m_pixelShader, nullptr));
  }

  DX12Shader::~DX12Shader()
  {
  }

  void DX12Shader::CreateRootSignature()
  {
    D3D12_STATIC_SAMPLER_DESC sampler = {};
    sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
    sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    sampler.MipLODBias = 0;
    sampler.MaxAnisotropy = 0;
    sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    sampler.MinLOD = 0.0f;
    sampler.MaxLOD = D3D12_FLOAT32_MAX;
    sampler.ShaderRegister = 0;
    sampler.RegisterSpace = 0;
    sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    std::vector<CD3DX12_DESCRIPTOR_RANGE1> ranges(m_rootParameters.size());
    std::vector<CD3DX12_ROOT_PARAMETER1> rootParams(m_rootParameters.size());

    auto cbvCount = 0;
    auto srvCount = 0;

    for (unsigned i = 0; i < m_rootParameters.size(); ++i)
    {
      auto count = m_rootParameters[i].type == D3D12_DESCRIPTOR_RANGE_TYPE_CBV ? cbvCount : srvCount;

      ranges[i].Init(m_rootParameters[i].type, 1, count, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
      rootParams[i].InitAsDescriptorTable(1, &ranges[i], m_rootParameters[i].visibility);
    
      cbvCount += 1 * m_rootParameters[i].type == D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
      srvCount += 1 * m_rootParameters[i].type == D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    }

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init_1_1(rootParams.size(), rootParams.data(), 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_1, &signature, &error));
    ThrowIfFailed(Device()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
  }

  void DX12Shader::ResetRootSignature()
  {
    m_rootSignature.Reset();
  }

  void DX12Shader::AddParameter(D3D12_DESCRIPTOR_RANGE_TYPE type, D3D12_SHADER_VISIBILITY visibility)
  {
    m_rootParameters.push_back({ type, visibility });
  }
}