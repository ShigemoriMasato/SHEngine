
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

RWStructuredBuffer<float3> positions : register(u0);
RWStructuredBuffer<Cell> cellIndices : register(u1);
RWStructuredBuffer<int> cellCounts : register(u2);

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
    
        for (j = 0; j < registerIndex; ++j)
        {
            if (indexRegister[j].cellIndex == cellIndex)
            {
                indexRegister[j].particleIndices[indexRegister[j].indexCount] = index * 32 + i;
                ++indexRegister[j].indexCount;
                find = true;
                break;
            }
        }

        if (!find)
        {
            indexRegister[registerIndex].cellIndex = cellIndex;
            indexRegister[registerIndex].particleIndices[0] = index * 32 + i;
            indexRegister[registerIndex].indexCount = 1;
            ++registerIndex;
        }
    }
    
    for (i = 0; i < registerIndex; ++i)
    {
        int cellIndex = indexRegister[i].cellIndex;
        int count = indexRegister[i].indexCount;
        int freeIndex;
        
        InterlockedAdd(cellCounts[cellIndex], count, freeIndex);
        if (freeIndex + count > particlePerCell)
        {
            count = max(0, particlePerCell - freeIndex);
        }
        
        for (int j = 0; j < count; ++j)
        {
            cellIndices[cellIndex].index[freeIndex + j] = indexRegister[i].particleIndices[j];
        }
    }
}
