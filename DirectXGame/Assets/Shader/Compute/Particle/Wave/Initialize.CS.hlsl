RWStructuredBuffer<int> localFreeList: register(u0);
RWStructuredBuffer<int> localFreeListIndex : register(u1);
RWStructuredBuffer<int> globalFreeList : register(u2);
RWStructuredBuffer<int> globalFreeListIndex : register(u3);
RWStructuredBuffer<int> indexList : register(u4);
RWStructuredBuffer<float16_t3> basePositions : register(u5);

cbuffer MaxParticle : register(b0) {
    uint maxParticleNum;
}

static const float16_t kMinValue16 = 6.103515625E-05;

[numthreads(1024, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID) {
    uint index = DTid.x;
    if (index >= maxParticleNum) return;

    uint freeIndex;
    InterlockedAdd(globalFreeListIndex[0], -1, freeIndex);
    if(freeIndex < 0) {
        InterlockedAdd(globalFreeListIndex[0], 1, freeIndex);
        return;
    }
    indexList[index] = globalFreeList[freeIndex];
    basePositions[index] = float16_t3(kMinValue16, kMinValue16, kMinValue16);
    
    localFreeList[index] = index;
    if(index == 0) {
        localFreeListIndex[0] = maxParticleNum - 1;
    }
}
