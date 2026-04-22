cbuffer ParticleNum : register(b0)
{
    uint maxNum;
};
cbuffer deltaTime : register(b1)
{
    float deltaTime;
};
cbuffer ID : register(b2)
{
    uint id;
};
cbuffer CSData : register(b3)
{
    float4x4 parentMatrix;
    float lifetime;
    float3 color;
    float3 fieldSize;
    uint textureID;
};

struct Wave
{
    float3 position;
    float speed;
    
    float3 color;
    float intensity;
    
    float lifetime;
    float decayRate;
    float maxlifetime;
    float thickness;
};
cbuffer WaveBuffer : register(b4)
{
    Wave waves[16];
}

RWStructuredBuffer<uint> freeList : register(u0);
RWStructuredBuffer<uint> freeListIndex : register(u1);
RWStructuredBuffer<float3> outPositions : register(u2);
RWStructuredBuffer<float4> colors : register(u3);
RWStructuredBuffer<uint> type : register(u4);
RWStructuredBuffer<float3> velocities : register(u5);
RWStructuredBuffer<float> lifetimes : register(u6);
RWStructuredBuffer<float3> positions : register(u7);
RWStructuredBuffer<uint> isUse : register(u8);

Texture2D<float4> textures[] : register(t8);
SamplerState gSampler : register(s0);

[numthreads(256, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;
    if (index >= maxNum)
    {
        return;
    }
    
    if (isUse[index] == 0)
    {
        type[index] = 0;
        return;
    }
    
    positions[index] += velocities[index] * deltaTime;
    
    //波の処理
    float3 pos = positions[index];
    float3 col = color;
    const int kWaveNum = 16;
    for (int i = 0; i < 16; ++i)
    {
        if (waves[i].lifetime >= waves[i].maxlifetime || waves[i].maxlifetime == 0)
        {
            continue;
        }
        
        float dist = length(pos.xz - waves[i].position.xz);
        float radius = waves[i].speed * waves[i].lifetime;
        float diff = dist - radius;
        float range = waves[i].thickness;
        
        if (diff > -range && diff < range)
        {
            float decay = 1.0f - (waves[i].lifetime / waves[i].maxlifetime);
            float intensity = (1.0f - abs(diff) / range) * waves[i].intensity * decay;
            pos += float3(0, intensity, 0);
            col += waves[i].color * intensity;
        }
    }
    
    //Textureを読み込んで、色がついていたら描画する
    float2 uv = (pos.xz + fieldSize.xz / 2.0f) / fieldSize.xz;
    float4 texColor = textures[textureID].SampleLevel(gSampler, uv, 0.0f);
    if (texColor.r > 0.1f)
    {
        type[index] = id;
    
        outPositions[index] = mul(float4(pos, 1.0f), parentMatrix).xyz;
        colors[index] = float4(col, float(lifetimes[index] / lifetime));     
    }
    else
    {
        type[index] = 0;
    }
    
    lifetimes[index] -= deltaTime;
    if (lifetimes[index] <= 0.0f)
    {
        uint freeIndex;
        InterlockedAdd(freeListIndex[0], 1, freeIndex);
        freeList[freeIndex + 1] = index;
        type[index] = 0;
        isUse[index] = 0;
    }
}