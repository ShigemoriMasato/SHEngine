cbuffer AllocateSize : register(b0)
{
    uint size;
};

cbuffer EmitterID : register(b1)
{
    uint emitterID;
}

StructuredBuffer<uint32_t> freeList : register(t0);

RWStructuredBuffer<int32_t> freeListIndex : register(u0);
RWStructuredBuffer<uint32_t> indexList : register(u1);
RWStructuredBuffer<uint32_t> parentIndices : register(u2);

static const uint loopCount = 32;

[numthreads(256, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x * loopCount;
    if (index >= size)
        return;
    
    int factLoopCount = min(loopCount, size - index);
    
    int poolBeginIndex;
    InterlockedAdd(freeListIndex[0], -factLoopCount, poolBeginIndex);
    
    //超過分を返す
    if (poolBeginIndex < loopCount)
    {
        int overCount = loopCount - poolBeginIndex;
        InterlockedAdd(freeListIndex[0], overCount);
        factLoopCount -= overCount;
    }
    
    int id, j = 0;
    for (int i = poolBeginIndex - factLoopCount; i <= poolBeginIndex; i++)
    {
        id = freeList[i];
        indexList[index + j] = id;
        parentIndices[id] = emitterID;
        j++;
    }
}
