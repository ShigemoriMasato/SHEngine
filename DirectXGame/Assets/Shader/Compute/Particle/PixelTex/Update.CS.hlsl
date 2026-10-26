cbuffer ParticleCount : register(b0)
{
    uint particleCount;
};

cbuffer Config : register(b1)
{
    float discrete;
    float range;
};

StructuredBuffer<uint> indexList : register(t0);
StructuredBuffer<float3> localPositions : register(t1);

RWStructuredBuffer<float3> positions : register(u0);
RWStructuredBuffer<float> scales : register(u1);
RWStructuredBuffer<float16_t4> color : register(u2);

uint Hash(uint x)
{
    x ^= x >> 16;
    x *= 0x7feb352d;
    x ^= x >> 15;
    x *= 0x846ca68b;
    x ^= x >> 16;
    return x;
}

float Rand(inout uint state)
{
    state = Hash(state);
    float ans = state / 4294967296.0;
    return ans * 2 - 1;
}

[numthreads(128, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    int index = DTid.x;
    if (index >= particleCount)
    {
        return;
    }
    
    float3 localPosition = localPositions[index];
    uint seed = asuint(localPosition.x + localPosition.y * discrete * discrete) * index;
    float3 discreteDirection = float3(Rand(seed) * range, Rand(seed) * range, Rand(seed));
    positions[indexList[index]] = discreteDirection * discrete + localPosition;
}
