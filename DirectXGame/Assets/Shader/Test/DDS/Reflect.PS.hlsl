
struct PSInput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
    float3 normal : NORMAL0;
    float3 worldPos : POSITION0;
};

struct PSOutput
{
    float4 color : SV_TARGET;
};

cbuffer Data : register(b0)
{
    float3 cameraPos;
    float strength;
}

TextureCube<float4> texture : register(t0);
SamplerState gSampler : register(s0);

PSOutput main(PSInput input)
{
    PSOutput output;
    
    float3 cameraToPosition = normalize(input.worldPos - cameraPos);
    float3 reflectedVector = reflect(cameraToPosition, normalize(input.normal));
    float4 enviromentColor = texture.Sample(gSampler, reflectedVector);
    output.color = lerp(float4(1.0f, 1.0f, 1.0f, 1.0f), enviromentColor, strength);
    
    return output;
}
