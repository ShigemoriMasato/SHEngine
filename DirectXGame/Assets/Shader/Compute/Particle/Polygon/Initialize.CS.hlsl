
cbuffer ParticleCount : register(b0)
{
    uint particleCount;
}

cbuffer ChanceListSize : register(b3)
{
    uint chanceListSize;
}

struct Vertex
{
    float32_t3 position;
    float32_t2 uv;
};

struct Polygon
{
    Vertex a;
    Vertex b;
    Vertex c;
    uint textureIndex;
};

StructuredBuffer<Polygon> polygons : register(t0);
StructuredBuffer<int> chanceList : register(t1);
StructuredBuffer<uint> indexList : register(t2);

Texture2D<float4> textures[] : register(t8);
SamplerState gSampler : register(s0);

RWStructuredBuffer<float3> tmp : register(u0);
RWStructuredBuffer<float> scales : register(u1);
RWStructuredBuffer<float16_t4> color : register(u2);
RWStructuredBuffer<float3> positions : register(u3);

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
    if (id >= particleCount)
    {
        return;
    }
    
    uint globalIndex = indexList[id];
    uint seed = 0x8b7c6ac1 ^ id * chanceListSize;
    
    //生成するポリゴンを抽選
    int chanceIndex = randu(seed, chanceListSize - 1);
    int polygonIndex = chanceList[chanceIndex];

    Polygon polygon = polygons[polygonIndex];

    //生成座標抽選
    float u = randf(seed);
    float v = randf(seed);

    if(u + v > 1.0f)
    {
        u = 1.0f - u;
        v = 1.0f - v;
    }

    float3 pos = polygon.a.position + u * (polygon.b.position - polygon.a.position) + v * (polygon.c.position - polygon.a.position);
    float2 uv = polygon.a.uv + u * (polygon.b.uv - polygon.a.uv) + v * (polygon.c.uv - polygon.a.uv);
    
    positions[id] = pos;
    float4 texColor = textures[polygon.textureIndex].SampleLevel(gSampler, uv, 0);
    if (texColor.a > 0.1f)
    {
        //cbvにするかも
        scales[indexList[id]] = 0.01f;
    }
    color[indexList[id]] = float16_t4(texColor.r, texColor.g, texColor.b, texColor.a);
}
