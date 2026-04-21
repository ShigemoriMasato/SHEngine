#include "Block.hlsli"

cbuffer VSData : register(b0)
{
    int2 size;                //ブロックの量
    int2 pad;
    float4x4 parentMatrix;
    float4x4 vpMatrix;
    float4 innerColor;
    float4 outerColor;
};

struct WaveData
{
    float strength;
    float speed;
    float tickness;
    float lifeTime;
    float lifeSpan;
    float2 generatePosition;
    float pad;
    float4 color;
    float3 direction;
    float pad2;
};

StructuredBuffer<WaveData> waveData : register(t0);

struct VSInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

VSOutput main(VSInput input, uint instance : SV_InstanceID)
{
    float2 start = -float2(size.x / 2, size.y / 2);
    float x = (instance % size.x) + start.x;
    float y = (instance / size.x) + start.y;
    
    float3 local = float3(x, 0.0f, y);
    float4 color = innerColor;
    
    const uint waveCount = 8;
    
    for (uint i = 0; i < waveCount; ++i)
    {
        if (waveData[i].strength <= 0.0f|| waveData[i].lifeTime > waveData[i].lifeSpan)
        {
            continue;
        }
        
        float rad = waveData[i].lifeTime * waveData[i].speed;
        float dist = distance(local.xz, waveData[i].generatePosition);
        float diff = rad - dist;
        
        if (abs(diff) < waveData[i].tickness)
        {
            float lifeStrength = saturate(1.0f - (waveData[i].lifeTime / waveData[i].lifeSpan));
            float fact = saturate((waveData[i].tickness - diff) / waveData[i].tickness) * waveData[i].strength * lifeStrength;
            
            float3 adjustDirforScale;
            adjustDirforScale.x = waveData[i].direction.x / parentMatrix[0][0];
            adjustDirforScale.y = waveData[i].direction.y / parentMatrix[1][1];
            adjustDirforScale.z = waveData[i].direction.z / parentMatrix[2][2];
            
            local += waveData[i].direction * fact * adjustDirforScale;
            color += waveData[i].color * fact * 0.5f;
        }
    }
    
    float4 pos = float4(local + input.position.xyz, 1.0f);
    
    VSOutput output;
    output.position = mul(pos, mul(parentMatrix, vpMatrix));
    output.texCoord = input.texcoord;
    output.normal = mul(input.normal, (float3x3) parentMatrix);
    output.color = color;
    output.outlineColor = outerColor;
    
    return output;
}
