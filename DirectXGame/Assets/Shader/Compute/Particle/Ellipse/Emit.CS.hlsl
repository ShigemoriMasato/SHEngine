RWStructuredBuffer<int> freeList : register(u0);
RWStructuredBuffer<int> freeListIndex : register(u1);
RWStructuredBuffer<float> currentTimes : register(u2);
RWStructuredBuffer<float> lifeTimes : register(u3);
RWStructuredBuffer<float32_t3> positions : register(u4);
RWStructuredBuffer<float16_t4> colors : register(u5);
RWStructuredBuffer<float32_t3> velocities : register(u6);

StructuredBuffer<int> indexList : register(t0);

cbuffer Config : register(b0)
{
    float2 posA;
    float2 posB;
    float z;
    float radius;
    float lifeTime;
    float initialSpeed;
    float4 color;
    float4x4 transform;
    uint randSeed;
    uint emitCount;
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

static const float PI = 3.1415926535897932384626433832795;
static const float MIN_FLT = 1.175494351e-38F;

[numthreads(128, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint id = DTid.x;
    if (id >= emitCount)
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
    
    float t = randf(seed);

    float rad = radius * sqrt(1.0f - (2.0f * t - 1.0f) * (2.0f * t - 1.0f));

    float theta = randf(seed) * PI * 2.0f;
    float r = rad * sqrt(randf(seed));

    float2 velocity = float2(cos(theta),sin(theta)) * r;
    
    velocities[index] = mul(float3(velocity, 0.0f), (float3x3)transform).xyz * initialSpeed;
    currentTimes[index] = 0.0f;
    lifeTimes[index] = lifeTime;
    
    positions[globalIndex] = mul(float4(lerp(posA, posB, t), z, 1.0f), transform).xyz;
    colors[globalIndex] = float16_t4(color.rgb, 1.0f);
}
