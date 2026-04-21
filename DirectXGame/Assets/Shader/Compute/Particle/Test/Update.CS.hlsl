cbuffer ParticleNum : register(b0)
{
    uint maxNum;
};
cbuffer deltaTime : register(b1)
{
    float deltaTime;
};
cbuffer Size : register(b2)
{
    float size;
};
cbuffer Camera : register(b3)
{
    float4x4 vpMatrix;
};

RWStructuredBuffer<float3> positions : register(u0);
RWStructuredBuffer<float3> velocities : register(u1);
RWStructuredBuffer<float> lifetimes : register(u2);
RWStructuredBuffer<float4x4> wvp : register(u3);

RWStructuredBuffer<uint> freeList : register(u4);
RWStructuredBuffer<uint> freeListIndex : register(u5);

[numthreads(1024, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;
    if (index >= maxNum)
    {
        return;
    }
    
    if (lifetimes[index] <= 0.0f)
    {
        wvp[index] = float4x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
        return;
    }
    
    positions[index] += velocities[index] * deltaTime;
    
    float4x4 world = float4x4(
        size, 0, 0, 0,
        0, size, 0, 0,
        0, 0, size, 0,
        positions[index].x, positions[index].y, positions[index].z, 1
    );
    wvp[index] = mul(world, vpMatrix);
    
    lifetimes[index] -= deltaTime;
    if(lifetimes[index] <= 0.0f)
    {
        uint freeIndex;
        InterlockedAdd(freeListIndex[0], 1, freeIndex);
        freeList[freeIndex + 1] = index;
    }
}