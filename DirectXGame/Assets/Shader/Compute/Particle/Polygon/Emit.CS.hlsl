RWStructuredBuffer<int> freeList : register(u0);
RWStructuredBuffer<int> freeListIndex : register(u1);
RWStructuredBuffer<float32_t3> positions : register(u2);
RWStructuredBuffer<float16_t4> colors : register(u3);
RWStructuredBuffer<float> currentTime : register(u4);
RWStructuredBuffer<float32_t3> basePositions : register(u5);
RWStructuredBuffer<float32_t3> velocities : register(u6);

struct Polygon {
    float32_t3 a;
    float32_t3 b;
    float32_t3 c;
};
StructuredBuffer<Polygon> polygons : register(t0);
StructuredBuffer<int> chanceList : register(t1);
StructuredBuffer<int> indexList : register(t2);

cbuffer EmitNum : register(b0) {
    uint emitNum;
}

cbuffer Color : register(b1){
    float32_t4 color;
}

cbuffer WorldMatrix : register(b2){
    float32_t4x4 worldMatrix;
}

cbuffer ChanceListSize : register(b3) {
    uint chanceListSize;
}

cbuffer RandSeed : register(b4) {
    uint randSeed;
}

cbuffer speed : register(b5)
{
    float speed;
}

uint Hash(uint x)
{
    x ^= x >> 16;
    x *= 0x7feb352d;
    x ^= x >> 15;
    x *= 0x846ca68b;
    x ^= x >> 16;
    return x;
}

float randf(inout uint state)
{
    state = Hash(state);
    return state / 4294967296.0;
}

uint randu(inout uint state, uint max)
{
    return (uint)(randf(state) * max);
}

[numthreads(128, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint id = DTid.x;
    if (id >= emitNum)
    {
        return;
    }

    uint freeIndex = 0;
    InterlockedAdd(freeListIndex[0], -1, freeIndex);
    if(freeIndex < 0)
    {
        InterlockedAdd(freeListIndex[0], 1);
        return;
    }
    
    uint index = freeList[freeIndex];
    uint globalIndex = indexList[index];
    uint seed = randSeed ^ id;
    
    //生成するポリゴンを抽選
    int chanceIndex = randu(seed, chanceListSize - 1);
    int polygonIndex = chanceList[chanceIndex];

    Polygon polygon = polygons[polygonIndex];

    //生成座標抽選
    float u = randf(seed);
    float v = randf(seed);
    float s = sqrt(u);

    if(u + v > 1.0f)
    {
        u = 1.0f - u;
        v = 1.0f - v;
    }

    float3 p = polygon.a + u * (polygon.b - polygon.a) + v * (polygon.c - polygon.a);
    
    currentTime[index] = 0.0f;

    float3 pos = mul(float4(p, 1.0f), worldMatrix).xyz;
    basePositions[index] = pos;
    velocities[index] = normalize(float3(randf(seed) - 0.5f, randf(seed) - 0.5f, randf(seed) - 0.5f)) * speed;
    positions[globalIndex] = pos;
    colors[globalIndex] = float16_t4(color.rgb, 1.0f);
}
