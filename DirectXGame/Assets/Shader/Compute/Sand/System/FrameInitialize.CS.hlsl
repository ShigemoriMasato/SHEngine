
cbuffer GridConfig : register(b0)
{
    float3 gridOrigin;
    float scale;
    
    int width;
    int height;
    int depth;
    int cellCount;
};

RWStructuredBuffer<uint> particleCountInCell : register(u0);
RWStructuredBuffer<uint> indexWriteBuffer : register(u1);

static const int loopCount = 8;

[numthreads(128, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x * loopCount;
    if (index >= cellCount)
    {
        return;
    }
    
    particleCountInCell[index] = 0;
    indexWriteBuffer[index] = 0;
}
