RWStructuredBuffer<uint> freeList : register(u0);
RWStructuredBuffer<uint> freeListIndex : register(u1);
RWStructuredBuffer<float> currentTime : register(u2);
RWStructuredBuffer<float3> position : register(u3);
RWStructuredBuffer<float16_t4> color : register(u4);
RWStructuredBuffer<float3> velocities : register(u5);
RWStructuredBuffer<uint> isFall : register(u6);

StructuredBuffer<int> indexList : register(t0);
StructuredBuffer<float3> basePositions : register(t1);

cbuffer MaxParticle : register(b0)
{
    uint maxParticleNum;
}

cbuffer LifeTime : register(b1)
{
    float lifeTime;
}

cbuffer deltaTime : register(b2)
{
    float deltaTime;
}

cbuffer Gravity : register(b3)
{
    float3 gravity;
}

[numthreads(128, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;
    if (index >= maxParticleNum)
        return;
    
    int globalIndex = indexList[index];
    float3 pos = position[globalIndex];

    //Positionが初期値のままなら何もしない
    static const float minValue = 1.175494351E-38;
    if (pos.x == minValue)
        return;
    
    if (isFall[index] == 1)
    {
        velocities[index] += gravity * deltaTime;
        position[globalIndex] += velocities[index] * deltaTime;
        currentTime[index] += deltaTime;
    }
    
    if (currentTime[index] >= lifeTime)
    {
        uint freeIndex;
        InterlockedAdd(freeListIndex[0], 1, freeIndex);
        freeList[freeIndex + 1] = index;
        position[globalIndex] = float3(minValue, minValue, minValue);
        isFall[index] = 0;
    }
}
