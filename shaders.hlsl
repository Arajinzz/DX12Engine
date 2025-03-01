cbuffer SceneConstantBuffer : register(b0)
{
    float4 color;
    float4 padding[15];
};

struct PSInput
{
    float4 position : SV_POSITION;
};

PSInput VSMain(float4 position : POSITION)
{
    PSInput result;

    //float4 pos = float4(position.xyz, 1.0);
    result.position = position;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    float2 color = input.position.xy / float2(1280, 720);
    return float4(color, 0.0, 1.0);
