cbuffer InitData : register(b0)
{
    float3 position;
    float lifeTime;
    float3 velocity;
};
cbuffer ParticleNum : register(b1)
{
    uint maxNum;
};
cbuffer EmitConfig : register(b2)
{
    int emitCount;
    int isEmit;
};
cbuffer Seed : register(b3)
{
    uint seed;
};

RWStructuredBuffer<uint> freeList : register(u0);
RWStructuredBuffer<uint> freeListIndex : register(u1);
RWStructuredBuffer<float3> positions : register(u2);
RWStructuredBuffer<float3> velocities : register(u3);
RWStructuredBuffer<float> lifetimes : register(u4);

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

[numthreads(1024, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    if (isEmit == 0)
    {
        return;
    }
    
    uint index = DTid.x;
    if (index >= emitCount)
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
    
    uint state = seed ^ index;
    
    uint particleIndex = freeList[freeIndex];
    positions[particleIndex] = position;
    velocities[particleIndex] = normalize(float3(Rand(state) * 2 - 1, Rand(state) * 2 - 1, Rand(state) * 2 - 1));
    lifetimes[particleIndex] = lifeTime;
}