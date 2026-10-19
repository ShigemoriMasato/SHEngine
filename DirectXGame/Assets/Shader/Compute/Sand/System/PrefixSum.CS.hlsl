cbuffer GridConfig : register(b0)
{
    float3 gridOrigin;
    float scale;
    
    int width;
    int height;
    int depth;
    int cellCount;
};

cbuffer GridBlock : register(b1)
{
    int cellCountPerBlock;
}

StructuredBuffer<uint> BlockNumber : register(t0);
StructuredBuffer<uint> particleCountInCell : register(t1);

RWStructuredBuffer<uint> beginIndexForCell : register(u0);

[numthreads(128, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;
    if (index >= cellCount)
    {
        return;
    }
    
    int i, ans = 0;
    int underBlockCount = index / cellCountPerBlock;
    int indexForCurrentBlock = index % cellCountPerBlock;
    int beginIndexForCellCount = cellCountPerBlock * underBlockCount;
    
    for (i = 0; i < underBlockCount; ++i)
    {
        ans += BlockNumber[i];
    }

    for (i = 0; i < indexForCurrentBlock; ++i)
    {
        ans += particleCountInCell[beginIndexForCellCount + i];
    }

    beginIndexForCell[index] = ans;
}
