#include "stdafx.h"
#include "DX12Cube.h"

#include "Core/DX12Device.h"
#include "Core/DXApplicationHelper.h"
#include "Core/Application.h"


namespace Core
{
  DX12Cube::DX12Cube(unsigned viewportWidth, unsigned viewportHeight, float padding)
    : m_commandList(nullptr)
    , m_viewport(0.0f, 0.0f, static_cast<float>(viewportWidth), static_cast<float>(viewportHeight))
    , m_scissorRect(0, 0, static_cast<LONG>(viewportWidth), static_cast<LONG>(viewportHeight))
  {
    // Create the command list.
    // the class should add command list automatically to CommandQueue
    m_commandList = std::make_unique<DX12CommandList>();

    // Create an empty root signature.
    {
      CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
      rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

      ComPtr<ID3DBlob> signature;
      ComPtr<ID3DBlob> error;
      ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
      ThrowIfFailed(Device()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
    }

    ComPtr<ID3DBlob> vertexShader;
    ComPtr<ID3DBlob> pixelShader;
    ComPtr<ID3DBlob> errorBlob;
    // Enable better shader debugging with the graphics debugging tools.
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;

    auto GetAssetFullPath = [&](std::wstring assetName) {
      WCHAR assetsPath[512];
      GetAssetsPath(assetsPath, _countof(assetsPath));
      return std::wstring(assetsPath) + assetName;
    };

    ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"shaders.hlsl").c_str(), nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, &errorBlob));
    ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"shaders.hlsl").c_str(), nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));
    // Define the vertex input layout.
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    // Describe and create the graphics pipeline state object (PSO).
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
    psoDesc.pRootSignature = m_rootSignature.Get();
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable = TRUE;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    psoDesc.SampleDesc.Count = 1;
    ThrowIfFailed(Device()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));

    {
      // Hack
      float aspectRatio = (1280.0 / 720.0);
      float vertexPositionX = 0.25 / aspectRatio;
      float vertexPosition = 0.25;
      Vertex cubeVertices[] =
      {
          { { -vertexPositionX + padding, -vertexPosition ,  vertexPosition }, { 1.0f, 0.0f, 0.0f, 1.0f } },
          { {  vertexPositionX + padding, -vertexPosition,  vertexPosition }, { 1.0f, 0.0f, 0.0f, 1.0f } },
          { {  vertexPositionX + padding,  vertexPosition,  vertexPosition }, { 0.0f, 1.0f, 0.0f, 1.0f } },
          { { -vertexPositionX + padding,  vertexPosition,  vertexPosition }, { 0.0f, 1.0f, 0.0f, 1.0f } },

          { { -vertexPositionX + padding, -vertexPosition, -vertexPosition }, { 1.0f, 0.0f, 0.0f, 1.0f } },
          { {  vertexPositionX + padding, -vertexPosition, -vertexPosition }, { 1.0f, 0.0f, 0.0f, 1.0f } },
          { {  vertexPositionX + padding,  vertexPosition, -vertexPosition }, { 0.0f, 1.0f, 0.0f, 1.0f } },
          { { -vertexPositionX + padding,  vertexPosition, -vertexPosition }, { 0.0f, 1.0f, 0.0f, 1.0f } },
      };
      // since array is on the stack it can deduce the size
      const unsigned vertexBufferSize = sizeof(cubeVertices);

      // Note: using upload heaps to transfer static data like vert buffers is not 
      // recommended. Every time the GPU needs it, the upload heap will be marshalled 
      // over. Please read up on Default Heap usage. An upload heap is used here for 
      // code simplicity and because there are very few verts to actually transfer.
      auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
      auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
      ThrowIfFailed(Device()->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_vertexBuffer)));

      // Copy the triangle data to the vertex buffer.
      UINT8* pVertexDataBegin;
      CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
      ThrowIfFailed(m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
      memcpy(pVertexDataBegin, cubeVertices, vertexBufferSize);
      m_vertexBuffer->Unmap(0, nullptr);

      // Initialize the vertex buffer view.
      m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
      m_vertexBufferView.StrideInBytes = sizeof(Vertex);
      m_vertexBufferView.SizeInBytes = vertexBufferSize;
    }

    {
      uint16_t indicies[] = {
        // front face
        0, 3, 2,
        0, 2, 1,

        // back face
        4, 5, 6,
        4, 6, 7,

        // left face
        0, 1, 5,
        0, 5, 4,

        // right face
        3, 7, 6,
        3, 6, 2,

        // top face
        1, 2, 6,
        1, 6, 5,

        // bottom face
        0, 4, 7,
        0, 7, 3
      };

      const unsigned indexBufferSize = sizeof(indicies);

      auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
      auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize);
      ThrowIfFailed(Device()->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_indexBuffer)));

      // Copy the index data to the index buffer.
      UINT8* pIndexDataBegin;
      CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
      ThrowIfFailed(m_indexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pIndexDataBegin)));
      memcpy(pIndexDataBegin, indicies, indexBufferSize);
      m_indexBuffer->Unmap(0, nullptr);

      // Initialize the index buffer view.
      m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
      m_indexBufferView.SizeInBytes = indexBufferSize;
      m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
    }

    // not good
    m_commandList->Close();
  }

  DX12Cube::~DX12Cube()
  {
  }

  // just a triangle for now
  void DX12Cube::Draw(DX12Heap* rtvHeap, DX12Heap* dsvHeap, unsigned frameIndex)
  {
    // 1 allocator
    m_commandList->Reset(frameIndex, m_pipelineState.Get());
    // Set necessary state.
    m_commandList->SetRootSignature(m_rootSignature.Get());

    // these must be done in the same commandlist as drawing
    // because they set a state for rendering
    // and states they reset between command lists
    m_commandList->Get()->RSSetViewports(1, &m_viewport);
    m_commandList->Get()->RSSetScissorRects(1, &m_scissorRect);
    auto rtvHandle = rtvHeap->GetOffsetHandle(frameIndex);
    auto dsvHandle = dsvHeap->GetOffsetHandle(0);
    m_commandList->Get()->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

    /*m_commandList->SetDescriptorHeap(m_cbvHeap.get());
    auto handle = m_cbvHeap->Get()->GetGPUDescriptorHandleForHeapStart();
    m_commandList->Get()->SetGraphicsRootDescriptorTable(0, handle);*/
    m_commandList->Get()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_commandList->Get()->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    m_commandList->Get()->IASetIndexBuffer(&m_indexBufferView);
    m_commandList->Get()->DrawIndexedInstanced(36, UINT(36 / 3), 0, 0, 0);
    
    m_commandList->Close();
  }
}