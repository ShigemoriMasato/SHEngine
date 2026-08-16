struct Sphere
{
    float3 position;
    float radius;
};

RWStructuredBuffer<uint> isFall : register(u0);
RWStructuredBuffer<float3> velocities : register(u1);
RWStructuredBuffer<float16_t4> color : register(u2);

StructuredBuffer<float3> position : register(t0);
StructuredBuffer<Sphere> spheres : register(t1);
StructuredBuffer<int> indexList : register(t2);

cbuffer MaxParticle : register(b0)
{
    uint maxParticleNum;
}

cbuffer SphereNum : register(b1)
{
    uint sphereNum;
}

cbuffer Seed : register(b2)
{
    uint rawSeed;
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

[numthreads(128, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint id = DTid.x;
    if (id >= maxParticleNum)
        return;
    
    uint seed = rawSeed ^ id;
    int index = indexList[id];
    float3 pos = position[index];
    
    for (uint i = 0; i < sphereNum; i++)
    {
        Sphere sphere = spheres[i];
        float dist = distance(pos, sphere.position);
        if (dist < sphere.radius)
        {
            isFall[id] = 1;
            velocities[id] = normalize(float3(randf(seed), randf(seed), randf(seed)));
            color[index] = float16_t4(1.0, 1.0, 1.0, 1.0);
            return;
        }
    }
}
