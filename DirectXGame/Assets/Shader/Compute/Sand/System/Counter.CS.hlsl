
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
    int first;
    int second;
};

StructuredBuffer<float3> positions : register(t0);

RWStructuredBuffer<uint> particleCountInCell : register(u0);

static const int loopCount = 8;

[numthreads(128, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x * loopCount;
    if (index >= particleCount)
    {
        return;
    }
    
    bool find;
    int i, j, registerIndex = 0;
    Pair beginIndex[loopCount];
    
    int factLoopCount = min(loopCount, particleCount - index);
    
    //Atomicを使うと競合が発生するので、loopCount分を処理してからあとでまとめて加算する
    for (i = 0; i < factLoopCount; ++i)
    {
        find = false;
        float3 localPosition = positions[index + i] - gridOrigin;
        int cellX = (int) (localPosition.x / scale);
        int cellY = (int) (localPosition.y / scale);
        int cellZ = (int) (localPosition.z / scale);
    
        if (cellX < 0 || cellX >= width || cellY < 0 || cellY >= height || cellZ < 0 || cellZ >= depth)
        {
            continue;
        }
        
        int cellIndex = cellX + cellY * width + cellZ * width * height;
        
        // すでに登録済みのセルかどうかを確認
        for (j = 0; j < registerIndex; ++j)
        {
            if (beginIndex[j].first == cellIndex)
            {
                ++beginIndex[j].second;
                find = true;
                break;
            }
        }

        //登録済みでない場合は登録し、登録した数(registerIndex)を1増やす
        if (!find)
        {
            beginIndex[registerIndex].first = cellIndex;
            beginIndex[registerIndex].second = 1;
            ++registerIndex;
        }
    }
    
    for (i = 0; i < registerIndex; ++i)
    {
        InterlockedAdd(particleCountInCell[beginIndex[i].first], beginIndex[i].second);
    }
}
