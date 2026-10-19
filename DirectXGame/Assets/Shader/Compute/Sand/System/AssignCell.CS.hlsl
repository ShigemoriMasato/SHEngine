
static const int loopCount = 8;

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

// セル番号でソートしたパーティクルのインデックスを格納するバッファ
RWStructuredBuffer<uint> particleIndexForCell : register(u0);
// 書き込む際、別スレッドと書き込み位置が競合しないようにするバッファ (毎フレームゼロ初期化すること)
RWStructuredBuffer<uint> indexWriteBuffer : register(u1);

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
        float3 localPosition = positions[index + i] - gridOrigin;
    
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
        InterlockedAdd(indexWriteBuffer[pairs[i].cellIndex], pairs[i].indexCount, freeIndex);
        
        int beginIndex = beginIndexForCell[pairs[i].cellIndex] + freeIndex;
        
        for (j = 0; j < pairs[i].indexCount; ++j)
        {
            particleIndexForCell[beginIndex + j] = pairs[i].indices[j];
        }
    }
}
