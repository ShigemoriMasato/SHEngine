
static const int loopCount = 32;

cbuffer Counter : register(b0)
{
    uint particleCount;
};

cbuffer GridConfig : register(b1)
{
    float scale;
    float3 leftBottomBack;
    
    int width;
    int height;
    int depth;
    int cellCount;
};

struct Pair
{
    int cellIndex;
    int indexCount;
    int indices[loopCount];
};

StructuredBuffer<float3> positions : register(t0);
StructuredBuffer<uint> beginIndexForCell : register(t1);

RWStructuredBuffer<uint> particleIndexForCell : register(u0);
RWStructuredBuffer<uint> registerCount : register(u1);

[numthreads(128, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x * loopCount;
    if (index >= particleCount)
    {
        return;
    }
    
    int i, j;
    int factLoopCount = min(loopCount, particleCount - index);
    Pair pairs[loopCount];
    int pairRegisterIndex = 0;
    bool find;
    
    for (i = 0; i < factLoopCount; ++i)
    {
        find = false;
        float3 localPosition = positions[index] + leftBottomBack;
    
        int3 cell;
        cell.x = (int) (localPosition.x / scale);
        cell.y = (int) (localPosition.y / scale);
        cell.z = (int) (localPosition.z / scale);
    
        int myCellIndex = cell.x + cell.y * width + cell.z * width * height;
        
        
        for (j = 0; j < pairRegisterIndex; ++j)
        {
            if (pairs[j].cellIndex == myCellIndex)
            {
                pairs[j].indices[pairs[j].indexCount] = index;
                pairs[j].indexCount++;
                find = true;
            }
        }

        if (!find)
        {
            pairs[pairRegisterIndex].cellIndex = myCellIndex;
            pairs[pairRegisterIndex].indexCount = 1;
            pairs[pairRegisterIndex].indices[0] = index;
            pairRegisterIndex++;
        }
    }
    
    for (i = 0; i < pairRegisterIndex; i++)
    {
        int freeIndex;
        InterlockedAdd(registerCount[pairs[i].cellIndex], pairs[i].indexCount, freeIndex);
        
        int beginIndex = beginIndexForCell[pairs[i].cellIndex] + freeIndex;
        
        for (j = 0; j < pairs[i].indexCount; ++j)
        {
            particleIndexForCell[beginIndex + j] = pairs[i].indices[j];
        }
    }
}
