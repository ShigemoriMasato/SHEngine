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
cbuffer lifeTime : register(b3)
{
    float lifetime;
};

RWStructuredBuffer<uint> freeList : register(u0);
RWStructuredBuffer<uint> freeListIndex : register(u1);
RWStructuredBuffer<float3> positions : register(u2);
RWStructuredBuffer<float4> color : register(u3);
RWStructuredBuffer<uint> type : register(u4);
RWStructuredBuffer<float3> velocities : register(u5);
RWStructuredBuffer<float> lifetimes : register(u6);

[numthreads(1024, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;
    if (index >= maxNum)
    {
        return;
    }
    
    if (type[index] != id)
    {
        return;
    }
    
    positions[index] += velocities[index] * deltaTime;
    color[index] = float4(1, 1, 1, float(lifetimes[index] / lifetime));
    
    lifetimes[index] -= deltaTime;
    if(lifetimes[index] <= 0.0f)
    {
        uint freeIndex;
        InterlockedAdd(freeListIndex[0], 1, freeIndex);
        freeList[freeIndex + 1] = index;
        type[index] = 0;
    }
}