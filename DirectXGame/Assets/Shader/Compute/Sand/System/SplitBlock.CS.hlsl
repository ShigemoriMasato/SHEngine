cbuffer GridConfig : register(b0)
{
    float3 gridOrigin;
    float scale;
    
    int width;
    int height;
    int depth;
    int cellCount;
};

cbuffer CellCountPerBlock : register(b1)
{
    int cellCountPerBlock;
}

StructuredBuffer<uint> particleCountInCell : register(t0);
RWStructuredBuffer<uint> BlockNumber : register(u0);

[numthreads(128, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x * cellCountPerBlock;
    if (index >= cellCount)
    {
        return;
    }
    
    int i, ans = 0;
    int factLoopCount = min(cellCountPerBlock, cellCount - index);
    
    for (i = 0; i < factLoopCount; ++i)
    {
        ans += particleCountInCell[index + i];
    }

    BlockNumber[DTid.x] = ans;
}
