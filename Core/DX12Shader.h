#pragma once

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace Core
{
  // will contain root signature and shader compilation
  class DX12Shader
  {
  public:
    DX12Shader(const char* path);
    ~DX12Shader();

    void CreateRootSignature();
    void ResetRootSignature();
    void AddParameter(D3D12_DESCRIPTOR_RANGE_TYPE type, D3D12_SHADER_VISIBILITY visibility);

    ID3DBlob* GetVertexShader() { return m_vertexShader.Get(); }
    ID3DBlob* GetPixelShader() { return m_pixelShader.Get(); }
    ID3D12RootSignature* GetRootSignature() { return m_rootSignature.Get(); }

  private:
    ComPtr<ID3D12RootSignature> m_rootSignature;

    ComPtr<ID3DBlob> m_vertexShader;
    ComPtr<ID3DBlob> m_pixelShader;

    std::vector<CD3DX12_DESCRIPTOR_RANGE1> m_ranges;
    std::vector<CD3DX12_ROOT_PARAMETER1> m_rootParameters;

  private:
    DX12Shader(const DX12Shader&) = delete;
    DX12Shader& operator=(const DX12Shader&) = delete;
  };
}

