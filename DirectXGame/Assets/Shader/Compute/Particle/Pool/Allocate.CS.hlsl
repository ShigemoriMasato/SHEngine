cbuffer AllocateSize : register(b0)
{
    uint size;
};

RWStructuredBuffer<uint32_t> poolFreeList : register(u0);
RWStructuredBuffer<uint32_t> poolIndex : register(u1);

RWStructuredBuffer<uint32_t> outputFreeList : register(u2);
RWStructuredBuffer<uint32_t> outputIndex : register(u3);

static const uint loopCount = 16;

[numthreads(256, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x * loopCount;
    if (index >= size)
        return;
    
    int factLoopCount = min(loopCount, size - index);
    
    int poolBeginIndex;
    InterlockedAdd(poolIndex[0], -factLoopCount, poolBeginIndex);
    
    //プールが足りない場合は、プールのインデックスを戻す
    if (poolBeginIndex < 0)
    {
        InterlockedAdd(poolIndex[0], factLoopCount);
        return;
    }
    
    int outputBeginIndex;
    InterlockedAdd(outputIndex[0], factLoopCount, outputBeginIndex);
    
    //出力が足りない場合は、出力のインデックスを戻す
    if (outputBeginIndex + factLoopCount > size)
    {
        InterlockedAdd(outputIndex[0], -factLoopCount);
        InterlockedAdd(poolIndex[0], factLoopCount);
        return;
    }
    
    for (int i = 0; i < factLoopCount; i++)
    {
        outputFreeList[poolBeginIndex + i] = poolFreeList[index + i];
    }
}
