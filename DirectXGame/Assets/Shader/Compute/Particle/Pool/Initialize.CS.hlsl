RWStructuredBuffer<int32_t> freeList : register(u0);
RWStructuredBuffer<int32_t> freeListIndex : register(u1);
RWStructuredBuffer<float3> position : register(u2);

cbuffer ParticleNum : register(b0)
{
    uint32_t maxNum;
};

[numthreads(1024, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID, uint3 GroupId : SV_GroupID)
{
    uint index = DTid.x;
    if (index >= maxNum)
    {
        return;
    }
    
    //FreeListの初期化
    freeList[index] = maxNum - 1 - index;
    if (DTid.x == 0)//同時に書き込まないようにするため
    {
        freeListIndex[0] = maxNum - 1;
    }
    
    float32_t minValue = 1.175494351E-38;
    position[index] = float3(minValue, minValue, minValue);
}