cbuffer ParticleCount : register(b0)
{
    uint particleCount;
};

cbuffer Config : register(b1)
{
    float2 impactPos;
    float radius;
    float scatter;

    float minScatter;
    float heightRange;
    float heightOffset;
    float randForce;
    
    float randRange;
    float randHeightRange;
    uint seed;
};

StructuredBuffer<uint> indexList : register(t0);
StructuredBuffer<float2> uvs : register(t1);

RWStructuredBuffer<float3> position : register(u0);
RWStructuredBuffer<float> scales : register(u1);
RWStructuredBuffer<float16_t4> color : register(u2);

RWStructuredBuffer<float16_t3> velocities : register(u3);

static const float pie = 3.14159265358f;

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
    float value = state / 4294967296.0;
    return value * 2 - 1;
}

float4 ColorGradation(float t)
{
    t = saturate(t);
    if (t < 1.0 / 3.0)
    {
        // 赤 → 緑
        float u = t * 3.0;
        return float4(1.0 - u, u, 0.0, 1.0);
    }
    else if (t < 2.0 / 3.0)
    {
        // 緑 → 青
        float u = (t - 1.0 / 3.0) * 3.0;
        return float4(0.0, 1.0 - u, u, 1.0);
    }
    else
    {
        // 青 → 白
        float u = (t - 2.0 / 3.0) * 3.0;
        return float4(u, u, 1.0, 1.0);
    }
}

[numthreads(128, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;
    if (index >= particleCount)
        return;
    
    uint localIndex = indexList[index];
    uint seedValue = seed ^ index;
    float2 pos = position[localIndex].xz;

    float2 diff = pos - impactPos;
    float2 dir = normalize(diff);
    float len = length(diff);

    float speed = max(minScatter, (radius - len) * scatter);

    float2 vel = dir * speed;
    
    float distRatio = saturate(len / (radius * 3.f));

    float heightVel = (sin((len / radius) * pie - pie) * heightRange * (1.2f - distRatio) + heightOffset);

    float3 randVel = float3(randf(seedValue), randf(seedValue) * 0.3f, randf(seedValue)) * (randForce + randf(seedValue) * randRange);
    
    velocities[index] = (float16_t3(vel.x, heightVel, vel.y) + float16_t3(randVel));
     
    // デバッグ
    color[localIndex] = float16_t4(ColorGradation(distRatio)) * float16_t4(0.3f, 0.3f, 0.3f, 1.0f);
}
