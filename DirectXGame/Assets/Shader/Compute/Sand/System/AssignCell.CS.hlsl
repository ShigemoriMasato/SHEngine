
static const int loopCount = 32;

cbuffer Counter : register(b0)
{
    uint particleCount;
};

cbuffer GridConfig : register(b1)
{
    float3 gridOrigin;
    float scale;
    
    int width;
    int height;
    int depth;
    int particlePerCell;
    
    int cellCount;
};

struct Cell
{
    int index[4096];
};

struct Pair
{
    int cellIndex;
    int particleIndices[loopCount];
    int indexCount;
};

RWStructuredBuffer<uint> particleCountInCell : register(t1);

RWStructuredBuffer<float3> positions : register(u0);
RWStructuredBuffer<uint> particleIndexForCell : register(u1);

[numthreads(128, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;
    if (index * 32 >= particleCount)
    {
        return;
    }
    
    bool find;
    int i, j;
    int registerIndex = 0;
    Pair indexRegister[loopCount];
    
    int factLoopCount = min(loopCount, particleCount - index * 32);
    
    for (i = 0; i < factLoopCount; ++i)
    {
        find = false;
        float3 localPosition = positions[index * 32 + i] - gridOrigin;
        int cellX = (int) (localPosition.x / scale);
        int cellY = (int) (localPosition.y / scale);
        int cellZ = (int) (localPosition.z / scale);
    
        if (cellX < 0 || cellX >= width || cellY < 0 || cellY >= height || cellZ < 0 || cellZ >= depth)
        {
            continue;
        }
        
        int cellIndex = cellX + cellY * width + cellZ * width * height;
    }
    
    for (i = 0; i < registerIndex; ++i)
    {
        
    }
}
