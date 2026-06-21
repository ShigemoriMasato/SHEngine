cbuffer CSData : register(b0)
{
    float3 fieldSize;
    float speed;
    
    float3 color;
    float lifetime;
    
    int emitNum;
    uint seed;
    uint textureID;
};
cbuffer ID : register(b1)
{
    uint id;
};

RWStructuredBuffer<uint> freeList : register(u0);
RWStructuredBuffer<uint> freeListIndex : register(u1);
RWStructuredBuffer<uint> type : register(u2);
RWStructuredBuffer<float3> positions : register(u3);
RWStructuredBuffer<float3> velocities : register(u4);
RWStructuredBuffer<float> lifetimes : register(u5);
RWStructuredBuffer<int> isUse : register(u6);
RWStructuredBuffer<float4> colors : register(u7);

Texture2D<float4> textures[] : register(t8);
SamplerState gSampler : register(s0);

uint Hash(uint x)
{
    x ^= x >> 16;
    x *= 0x7feb352d;
    x ^= x >> 15;
    x *= 0x846ca68b;
    x ^= x >> 16;
    return x;
}

float Rand(inout uint state)
{
    state = Hash(state);
    return state / 4294967296.0;
}

[numthreads(64, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;
    if (index >= emitNum)
    {
        return;
    }
    
    uint state = seed ^ index;
    float3 position = float3(Rand(state), Rand(state), Rand(state));
    float2 uv = position.xz;
    float4 texColor = textures[textureID].SampleLevel(gSampler, uv, 0);
    // テクスチャの色が暗い場合はパーティクルを発生させない
    if (texColor.r < 0.1f)
    {
        return;
    }
    
    int freeIndex;
    InterlockedAdd(freeListIndex[0], -1, freeIndex);
    if (freeIndex < 0)
    {
        InterlockedAdd(freeListIndex[0], 1, freeIndex);
        return;
    }
    
    uint particleIndex = freeList[freeIndex];
    
    positions[particleIndex] = position * fieldSize - fieldSize * 0.5;
    velocities[particleIndex] = normalize(float3(Rand(state), Rand(state), Rand(state)) * fieldSize - fieldSize * 0.5) * speed;
    lifetimes[particleIndex] = lifetime;
    type[particleIndex] = id;
    isUse[particleIndex] = 1;
    colors[particleIndex] = float4(color.rgb * texColor.rgb, 1.0f);
}
