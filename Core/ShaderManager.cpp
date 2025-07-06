#include "stdafx.h"
#include "ShaderManager.h"

#include "Core/DX12Interface.h"

#include <filesystem>
#include <fstream>
#include <json.hpp>
using json = nlohmann::json;

namespace Core
{
  ShaderManager::ShaderManager()
    : m_PSOs()
  {
    RegisterShaders();
  }

  ShaderManager::~ShaderManager()
  {
    m_PSOs.clear();
  }

  void ShaderManager::RegisterShaders()
  {
    // TODO: handle errors
    auto configPath = std::filesystem::current_path().string() + "/Configs/Shaders.json";

    // read config file and parse it
    json configData = json::parse(std::ifstream(configPath))["Shaders"];

    // for now we have only Skybox shader, and model shader
    auto SkyboxShader = ShaderBlob(L"Shaders/skybox_shaders.hlsl", false);
    auto ModelShader = ShaderBlob(L"Shaders/shaders.hlsl", false);
    auto MipsCompute = ShaderBlob(L"Shaders/GenerateMips_CS.hlsl", true);
    // register to map
    m_PSOs[SkyboxShader.filename] = PSO();
    m_PSOs[ModelShader.filename] = PSO();
    m_PSOs[MipsCompute.filename] = PSO();
    // root signature it is shared
    ComPtr<ID3D12RootSignature> rootSig;

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

    m_PSOs[SkyboxShader.filename].m_rootSignature = rootSig;
    m_PSOs[ModelShader.filename].m_rootSignature = rootSig;

    // create pipeline state object
    // Define the vertex input layout.
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    // Describe and create the graphics pipeline state object (PSO).
    D3D12_GRAPHICS_PIPELINE_STATE_DESC skyboxPSODesc = {};
    skyboxPSODesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
    skyboxPSODesc.pRootSignature = rootSig.Get();
    skyboxPSODesc.VS = CD3DX12_SHADER_BYTECODE(SkyboxShader.vertexShader.Get());
    skyboxPSODesc.PS = CD3DX12_SHADER_BYTECODE(SkyboxShader.pixelShader.Get());
    skyboxPSODesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    skyboxPSODesc.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT; // front culling
    skyboxPSODesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    skyboxPSODesc.DepthStencilState.DepthEnable = false; // no depth
    skyboxPSODesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    skyboxPSODesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    skyboxPSODesc.DepthStencilState.StencilEnable = FALSE;
    skyboxPSODesc.SampleMask = UINT_MAX;
    skyboxPSODesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    skyboxPSODesc.NumRenderTargets = 1;
    skyboxPSODesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    skyboxPSODesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    skyboxPSODesc.SampleDesc.Count = 1;
    ThrowIfFailed(
      DX12Interface::Get().GetDevice()->CreateGraphicsPipelineState(&skyboxPSODesc, IID_PPV_ARGS(&m_PSOs[SkyboxShader.filename].m_pipelineState)));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC modelPSODesc = {};
    modelPSODesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
    modelPSODesc.pRootSignature = rootSig.Get();
    modelPSODesc.VS = CD3DX12_SHADER_BYTECODE(ModelShader.vertexShader.Get());
    modelPSODesc.PS = CD3DX12_SHADER_BYTECODE(ModelShader.pixelShader.Get());
    modelPSODesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    modelPSODesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK; // back culling
    modelPSODesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    modelPSODesc.DepthStencilState.DepthEnable = true; // enable depth
    modelPSODesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    modelPSODesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    modelPSODesc.DepthStencilState.StencilEnable = FALSE;
    modelPSODesc.SampleMask = UINT_MAX;
    modelPSODesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    modelPSODesc.NumRenderTargets = 1;
    modelPSODesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    modelPSODesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    modelPSODesc.SampleDesc.Count = 1;
    ThrowIfFailed(
      DX12Interface::Get().GetDevice()->CreateGraphicsPipelineState(&modelPSODesc, IID_PPV_ARGS(&m_PSOs[ModelShader.filename].m_pipelineState)));

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
    
    // Pipeline state descriptor
    D3D12_COMPUTE_PIPELINE_STATE_DESC computePsoDesc = {};
    computePsoDesc.pRootSignature = computeRootSig.Get();  // Root signature
    computePsoDesc.CS.pShaderBytecode = MipsCompute.computeShader->GetBufferPointer();
    computePsoDesc.CS.BytecodeLength = MipsCompute.computeShader->GetBufferSize();
    computePsoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE; // Default flag
    // set
    m_PSOs[MipsCompute.filename].m_rootSignature = computeRootSig;
    // Create the compute PSO
    DX12Interface::Get().GetDevice()->CreateComputePipelineState(&computePsoDesc, IID_PPV_ARGS(&m_PSOs[MipsCompute.filename].m_pipelineState));

  }
}
