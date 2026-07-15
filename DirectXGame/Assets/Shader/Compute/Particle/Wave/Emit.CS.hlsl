cbuffer CSData : register(b0)
{
    float3 fieldSize;
    float speed;
    
    float3 color;
    int emitNum;
    
    uint seed;
    uint textureID;
};
cbuffer LifeTime : register(b1)
{
    float lifetime;
};

RWStructuredBuffer<int> freeList : register(u0);
RWStructuredBuffer<int> freeListIndex : register(u1);
RWStructuredBuffer<float16_t3> basePositions : register(u2);
RWStructuredBuffer<float16_t4> colors : register(u3);
RWStructuredBuffer<float16_t3> velocities : register(u4);
RWStructuredBuffer<float16_t> currentTime : register(u5);


StructuredBuffer<int> indexList : register(t0);

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

[numthreads(128, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint id = DTid.x;
    if (id >= emitNum)
    {
        return;
    }
    
    uint state = seed ^ id;
    float3 position = float3(Rand(state), Rand(state), Rand(state));
    float2 uv = position.xz;
    uv.y = 1.0f - uv.y; // テクスチャのUVは左上が原点なので、Y軸を反転させる
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
    
    uint localIndex = freeList[freeIndex];
    uint index = indexList[localIndex];
    
    basePositions[index] = float16_t3(position * fieldSize - fieldSize * 0.5);
    velocities[index] = float16_t3(normalize(float32_t3(Rand(state), Rand(state), Rand(state)) * 2 - float3(1, 1, 1)) * speed);
    currentTime[index] = 0;
    colors[index] = float16_t4(clamp(float4(color.rgb * texColor.rgb, 1.0f), float4(0, 0, 0, 0), float4(1, 1, 1, 1)));
}
