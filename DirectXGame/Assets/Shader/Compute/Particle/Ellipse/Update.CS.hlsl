RWStructuredBuffer<uint> freeList : register(u0);
RWStructuredBuffer<uint> freeListIndex : register(u1);
RWStructuredBuffer<float> currentTime : register(u2);
RWStructuredBuffer<float3> velocities : register(u3);
RWStructuredBuffer<float3> position : register(u4);
RWStructuredBuffer<float16_t4> color : register(u5);

StructuredBuffer<int> indexList : register(t0);
StructuredBuffer<float> lifeTime : register(t1);

cbuffer MaxParticle : register(b0) {
    uint maxParticleNum;
}

cbuffer deltaTime : register(b1) {
    float deltaTime;
}


[numthreads(128, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID) {
    uint index = DTid.x;
    if (index >= maxParticleNum) return;

    int globalIndex = indexList[index];
    float3 pos = position[globalIndex];

    //Positionが初期値のままなら何もしない
    static const float minValue = 1.175494351E-38;
    if(pos.x == minValue) return;
    
    currentTime[index] += deltaTime;
    
    position[globalIndex] += velocities[index] * deltaTime;
    color[globalIndex].a = float16_t(1.0 - (currentTime[index] / lifeTime[index]));

    if (currentTime[index] >= lifeTime[index]) {
        uint freeIndex;
        InterlockedAdd(freeListIndex[0], 1, freeIndex);
        freeList[freeIndex + 1] = index;
        position[globalIndex] = float3(minValue, minValue, minValue);
    }
}
