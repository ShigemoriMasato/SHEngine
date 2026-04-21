#include "Block.hlsli"

struct VSInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

struct ParticleData
{
    float4x4 world;
    float4x4 vp;
    uint color;
    uint outlineColor;
    uint1x2 pad;
};

float4 ConvertColor(uint color)
{
    float4 result = float4(
        ((color >> 24) & 0xFF) / 255.0,
        ((color >> 16) & 0xFF) / 255.0,
        ((color >> 8) & 0xFF) / 255.0,
        (color & 0xFF) / 255.0);
    
    return result;
}

StructuredBuffer<ParticleData> data : register(t0);

VSOutput main(VSInput input, uint instance : SV_InstanceID)
{
    VSOutput output;
    output.position = mul(input.position, mul(data[instance].world, data[instance].vp));
    output.texCoord = input.texcoord;
    output.color = ConvertColor(data[instance].color);
    output.outlineColor = ConvertColor(data[instance].outlineColor);
    output.normal = normalize(mul(input.normal, (float3x3) data[instance].world));
    return output;
}
