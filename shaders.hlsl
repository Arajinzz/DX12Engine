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
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

float remap(float value, float inMin, float inMax, float outMin, float outMax)
{
    return outMin + (value - inMin) * (outMax - outMin) / (inMax - inMin);
}

PSInput VSMain(float4 position : POSITION, float3 normal : NORMAL, float2 uv : TEXCOORD)
{
    PSInput result;
    
    float4 pos = float4(position.xyz, 1.0);
    float4 worldPos = mul(pos, m_model);
    float4 viewPos = mul(worldPos, s_view);
    float4 clipPos = mul(viewPos, s_projection);
    
    result.position = clipPos;
    result.normal = mul(float4(normal, 0.0), m_model).xyz;
    result.uv = uv;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    float3 baseColor = float3(0.5f, 0.5f, 0.5f);
    float3 lighting = float3(0.0f, 0.0f, 0.0f);
    float3 normal = normalize(input.normal);
    float3 ambient = float3(0.5f, 0.5f, 0.5f);
    float3 skyColor = float3(0.0f, 0.3f, 0.6f);
    float3 groundColor = float3(0.6f, 0.3f, 0.1f);
    
    float hemiMix = remap(normal.y, -1.0, 1.0, 0.0, 1.0f);
    float3 hemi = lerp(groundColor, skyColor, hemiMix);
    
    // diffuse
    float3 lightDir = normalize(float3(1.0f, 1.0f, 1.0f));
    float3 lightColor = float3(1.0f, 1.0f, 0.9f);
    float diffuse = max(0.0f, dot(lightDir, normal)) * lightColor;
    
    lighting = ambient * 0.5 + hemi * 0.5 + diffuse * 0.5;
    
    float2 uv = frac(input.uv); // Ensure UVs stay in [0,1] range
    
    float4 color = g_texture.Sample(g_sampler, uv) * float4(lighting, 1.0);
  
    return color;
}