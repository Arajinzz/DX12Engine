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

namespace Graphics
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

  ID3D12RootSignature* PSOManager::GetRootSignature(const std::string& name)
  {
    auto shaderName = m_psoMap[name]->shaderName;
    auto shaderBlob = ShaderManager::Instance().GetShader(shaderName);
    return shaderBlob->rootSignature.Get();
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
      // set the shader name
      pso->shaderName = shader;

      if (type == "Graphics")
      { // graphics pso
        // maybe handle case sensitive problems and errors
        auto d3dCullMode = cullMode == "Back" ? D3D12_CULL_MODE_BACK : cullMode == "Front" ? D3D12_CULL_MODE_FRONT : D3D12_CULL_MODE_NONE;
        // TODO: handle other cases
        auto d3dDepthFunc = depthTesting == "Less" ? D3D12_COMPARISON_FUNC_LESS : D3D12_COMPARISON_FUNC_NONE;

        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
        psoDesc.pRootSignature = shaderBlob->rootSignature.Get();
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
        psoDesc.pRootSignature = shaderBlob->rootSignature.Get();  // Root signature
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
