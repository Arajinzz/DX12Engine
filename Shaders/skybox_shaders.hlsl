#define ROOTSIG \
  "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), " \
  "DescriptorTable(CBV(b0, numDescriptors=1)), " \
  "DescriptorTable(CBV(b1, numDescriptors=1)), " \
  "DescriptorTable(SRV(t0, numDescriptors=1, flags = DESCRIPTORS_VOLATILE | DATA_VOLATILE)), " \
  "StaticSampler(s0," \
        "addressU = TEXTURE_ADDRESS_CLAMP," \
        "addressV = TEXTURE_ADDRESS_CLAMP," \
        "addressW = TEXTURE_ADDRESS_CLAMP," \
        "filter = FILTER_ANISOTROPIC," \
        "maxAnisotropy = 16)"

cbuffer SceneConstantBuffer : register(b0)
{
    float4x4 s_model;
    float4x4 s_view;
    float4x4 s_projection;
    float4 s_padding[4];
};

cbuffer ModelConstantBuffer : register(b1)
{
    float4x4 m_model;
    float4x4 m_view;
    float4x4 m_projection;
    float4 m_padding[4];
};

TextureCube g_texture : register(t0);
SamplerState g_sampler : register(s0);

struct PSInput
{
    float4 position : SV_POSITION;
    float4 texCoord : TEXCOORD;
};

[RootSignature(ROOTSIG)]
PSInput VSMain(float4 position : POSITION, float3 normal : NORMAL, float2 uv : TEXCOORD)
{
    PSInput result;
    
    float4x4 viewNoTranslation = s_view;
    viewNoTranslation._41 = 0;
    viewNoTranslation._42 = 0;
    viewNoTranslation._43 = 0;
    
    float4 pos = float4(position.xyz, 1.0);
    float4 viewPos = mul(pos, viewNoTranslation);
    float4 clipPos = mul(viewPos, s_projection);
    
    result.position = clipPos;
    result.texCoord = pos;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return g_texture.Sample(g_sampler, normalize(input.texCoord));
}