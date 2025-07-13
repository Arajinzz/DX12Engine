#define ROOTSIG \
  "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), " \
  "DescriptorTable(SRV(t0, numDescriptors=1, flags = DESCRIPTORS_VOLATILE | DATA_VOLATILE)), " \
  "StaticSampler(s0," \
        "addressU = TEXTURE_ADDRESS_CLAMP," \
        "addressV = TEXTURE_ADDRESS_CLAMP," \
        "addressW = TEXTURE_ADDRESS_CLAMP," \
        "filter = FILTER_ANISOTROPIC," \
        "maxAnisotropy = 16)"

Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

struct PSInput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

[RootSignature(ROOTSIG)]
PSInput VSMain(float4 position : POSITION, float3 normal : NORMAL, float2 uv : TEXCOORD)
{
    PSInput result;
    
    result.position = position;
    result.normal = normal;
    result.uv = uv;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return g_texture.Sample(g_sampler, uv);
}