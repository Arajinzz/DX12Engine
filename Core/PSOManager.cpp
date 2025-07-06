#include "stdafx.h"
#include "PSOManager.h"
#include "Core/DX12Interface.h"
#include "Core/ShaderManager.h"

#include <filesystem>
#include <fstream>

// json
#include <json.hpp>
using json = nlohmann::json;

// TODO: move root signatures inside shaders

namespace Core
{
  PSOManager::PSOManager()
    : m_psoMap()
  {
    RegisterPSOs();
  }

  PSOManager::~PSOManager()
  {
    m_psoMap.clear();
  }

  void PSOManager::RegisterPSOs()
  {
    // create pipeline state object
    // Define the vertex input layout.
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    D3D12_STATIC_SAMPLER_DESC sampler = {};
    sampler.Filter = D3D12_FILTER_COMPARISON_ANISOTROPIC;
    sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.MipLODBias = 0;
    sampler.MaxAnisotropy = 1;
    sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    sampler.MinLOD = 0.0f;
    sampler.MaxLOD = D3D12_FLOAT32_MAX;
    sampler.ShaderRegister = 0;
    sampler.RegisterSpace = 0;
    sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    // TODO: move root signatures to shaders, use dxc???
    // root signature it is shared
    ComPtr<ID3D12RootSignature> rootSig;

    std::vector<CD3DX12_DESCRIPTOR_RANGE1> ranges(3);
    std::vector<CD3DX12_ROOT_PARAMETER1> rootParams(3);

    ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
    rootParams[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_VERTEX);
    ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
    rootParams[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_VERTEX);
    ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
    rootParams[2].InitAsDescriptorTable(1, &ranges[2], D3D12_SHADER_VISIBILITY_PIXEL);
    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init_1_1(
      static_cast<unsigned>(rootParams.size()), rootParams.data(), 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_1, &signature, &error));
    ThrowIfFailed(DX12Interface::Get().GetDevice()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSig)));

    // compute part
    ComPtr<ID3D12RootSignature> computeRootSig;

    CD3DX12_DESCRIPTOR_RANGE1 srcMip(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);
    CD3DX12_DESCRIPTOR_RANGE1 outMip(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 4, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);
    CD3DX12_ROOT_PARAMETER1 rootParameters[Core::NumRootParameters];
    rootParameters[Core::GenerateMipsCB].InitAsConstants(sizeof(SGenerateMipsCB) / 4, 0);
    rootParameters[Core::SrcMip].InitAsDescriptorTable(1, &srcMip);
    rootParameters[Core::OutMip].InitAsDescriptorTable(1, &outMip);
    CD3DX12_STATIC_SAMPLER_DESC linearClampSampler(
      0,
      D3D12_FILTER_MIN_MAG_MIP_LINEAR,
      D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
      D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
      D3D12_TEXTURE_ADDRESS_MODE_CLAMP
    );
    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC computeRootSignatureDesc(
      Core::NumRootParameters,
      rootParameters, 1, &linearClampSampler
    );
    ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&computeRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_1, &signature, &error));
    ThrowIfFailed(DX12Interface::Get().GetDevice()->CreateRootSignature(
      0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&computeRootSig)));

    // TODO: handle errors
    auto configPath = std::filesystem::current_path().string() + "/Configs/PipelineState.json";

    // read config file and parse it
    json configData = json::parse(std::ifstream(configPath))["PipelineStateObjects"];

    for (auto data : configData)
    {
      std::string name;
      std::string type;
      std::string shader;
      std::string cullMode;
      std::string depthTesting;

      data.at("Name").get_to(name);
      data.at("Type").get_to(type);
      data.at("Shader").get_to(shader);

      if (type != "Compute" && type == "Graphics")
      {
        data.at("CullMode").get_to(cullMode);
        data.at("DepthTesting").get_to(depthTesting);
      }

      // get the shader blob
      auto shaderBlob = ShaderManager::Instance().GetShader(shader);
      // the pso
      auto pso = std::make_shared<PSO>();

      if (type == "Graphics")
        pso->rootSignature = rootSig;
      else if (type == "Compute")
        pso->rootSignature = computeRootSig;

      if (type == "Graphics")
      { // graphics pso
        // maybe handle case sensitive problems and errors
        auto d3dCullMode = cullMode == "Back" ? D3D12_CULL_MODE_BACK : cullMode == "Front" ? D3D12_CULL_MODE_FRONT : D3D12_CULL_MODE_NONE;
        // TODO: handle other cases
        auto d3dDepthFunc = depthTesting == "Less" ? D3D12_COMPARISON_FUNC_LESS : D3D12_COMPARISON_FUNC_NONE;

        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
        psoDesc.pRootSignature = pso->rootSignature.Get();
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(shaderBlob->vertexShader.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(shaderBlob->pixelShader.Get());
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        psoDesc.RasterizerState.CullMode = d3dCullMode; // from config
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState.DepthEnable = depthTesting != "None"; // from config
        psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        psoDesc.DepthStencilState.DepthFunc = d3dDepthFunc; // from config
        psoDesc.DepthStencilState.StencilEnable = FALSE;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
        psoDesc.SampleDesc.Count = 1;
        ThrowIfFailed(
          DX12Interface::Get().GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso->pipelineState)));

      } else if (type == "Compute")
      { // compute pso
        // Pipeline state descriptor
        D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.pRootSignature = pso->rootSignature.Get();  // Root signature
        psoDesc.CS.pShaderBytecode = shaderBlob->computeShader->GetBufferPointer();
        psoDesc.CS.BytecodeLength = shaderBlob->computeShader->GetBufferSize();
        psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE; // Default flag
        // Create the compute PSO
        DX12Interface::Get().GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&pso->pipelineState));
      }

      if (type == "Graphics" || type == "Compute")
        m_psoMap[name] = pso; // store
    }

  }

}
