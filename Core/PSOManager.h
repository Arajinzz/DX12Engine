#pragma once

#include <unordered_map>

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

  struct PSO
  {
    ComPtr<ID3D12RootSignature> rootSignature;
    ComPtr<ID3D12PipelineState> pipelineState;

    ~PSO()
    {
      pipelineState.Reset();
      rootSignature.Reset();
    }
  };

  class PSOManager
  {
  public:
    static PSOManager& Instance()
    {
      static PSOManager instance;
      return instance;
    }
    ~PSOManager();

    ID3D12PipelineState* GetPSO(const std::string& name) { return m_psoMap[name]->pipelineState.Get(); }
    ID3D12RootSignature* GetRootSignature(const std::string& name) { return m_psoMap[name]->rootSignature.Get(); }

  private:
    void RegisterPSOs();

  private:
    std::unordered_map<std::string, std::shared_ptr<PSO>> m_psoMap;

  private:
    PSOManager();
    PSOManager(const PSOManager&) = delete;
    PSOManager& operator=(const PSOManager&) = delete;

  };

}
