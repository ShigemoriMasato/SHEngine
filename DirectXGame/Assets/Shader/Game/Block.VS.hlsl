#include "Block.hlsli"

struct VSInput
{
    float3 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

struct ParticleData
{
    float4x4 world;
    float4x4 wvp;
};

StructuredBuffer<ParticleData> data : register(t0);

VSOutput main(VSInput input, uint instance : SV_InstanceID)
{
    VSOutput output;
    output.position = mul(float4(input.position, 1.0f), data[instance].wvp);
    output.texCoord = input.texcoord;
    output.normal = mul(input.normal, (float3x3) data[instance].world);
    output.instanceID = instance;
    return output;
}
