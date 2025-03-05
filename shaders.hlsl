cbuffer SceneConstantBuffer : register(b0)
{
    float4x4 model;
    float4x4 view;
    float4x4 projection;
    float4 padding[4];
};

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

PSInput VSMain(float4 position : POSITION, float4 color : COLOR)
{
    PSInput result;
    
    float4 pos = float4(position.xyz, 1.0);
    float4 worldPos = mul(pos, model);
    float4 viewPos = mul(worldPos, view);
    float4 clipPos = mul(viewPos, projection);
    
    result.position = clipPos;
    result.color = color;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return input.color;
}