cbuffer Counter : register(b0)
{
    uint particleCount;
};

cbuffer Seed : register(b1)
{
    uint seed;
};

cbuffer Ball : register(b2)
{
    float3 center;
    float radius;
};

RWStructuredBuffer<float3> positions : register(u0);
RWStructuredBuffer<float3> forces : register(u1);
RWStructuredBuffer<float3> velocities : register(u2);

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

[numthreads(128, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;
    if (index >= particleCount)
    {
        return;
    }
    
    uint state = seed ^ index;
    float ballLen = randf(state) * radius;
    
    float phi = randf(state) * 6.28318530718; // 2 * PI
    float theta = randf(state) * 3.14159265359; // PI
    float3 dir = float3(cos(theta) * cos(phi), sin(theta), cos(theta) * sin(phi));
    
    positions[index] = center + dir * ballLen;
    forces[index] = float3(0, 0, 0);
    velocities[index] = float3(0, 0, 0);
}
