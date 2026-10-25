RWStructuredBuffer<uint32_t> freeList : register(u0);
RWStructuredBuffer<int32_t> freeListIndex : register(u1);
RWStructuredBuffer<float32_t3> position : register(u2);
RWStructuredBuffer<float32_t> scale : register(u3);
RWStructuredBuffer<float16_t4> color : register(u4);
RWStructuredBuffer<uint32_t> parentIndices : register(u5);

cbuffer ParticleNum : register(b0)
{
    uint32_t maxNum;
};

static const float32_t minValue = 1.175494351E-38;

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
    
    position[index] = float3(index * 1.0f, 0.0f, 0.0f);
    scale[index] = 0.0f;
    color[index] = float16_t4(1.0f, 1.0f, 1.0f, 1.0f);
    parentIndices[index] = 0xffffffff;
}