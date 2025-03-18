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

Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

PSInput VSMain(float4 position : POSITION, float4 color : COLOR, float2 uv : TEXCOORD)
{
    PSInput result;
    
    float4 pos = float4(position.xyz, 1.0);
    float4 worldPos = mul(pos, m_model);
    float4 viewPos = mul(worldPos, s_view);
    float4 clipPos = mul(viewPos, s_projection);
    
    result.position = clipPos;
    result.color = color;
    result.uv = uv;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return g_texture.Sample(g_sampler, input.uv) * input.color;
}