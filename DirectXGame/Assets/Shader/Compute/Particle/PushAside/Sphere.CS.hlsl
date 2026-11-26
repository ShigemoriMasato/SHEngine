cbuffer ParticleCount : register(b0)
{
    uint particleCount;
};

cbuffer Config : register(b1)
{
    int sphereCount;
    float ignoreSpeed;
}

cbuffer DeltaTime : register(b2)
{
    float deltaTime;
}

struct Sphere
{
    float3 center;
    float radius;
}

StructuredBuffer<uint> indexList : register(t0);
StructuredBuffer<Sphere> spheres : register(t1);

RWStructuredBuffer<float3> positions : register(u0);
RWStructuredBuffer<float3> rotations : register(u1);
RWStructuredBuffer<float> scales : register(u2);
RWStructuredBuffer<float16_t4> color : register(u3);

[numthreads(128, 1, 1)]
int main(uint3 DTid : SV_DispatchThreadID)
{
    int index = DTid.x;
    if (index >= particleCount)
    {
        return 0;
    }

    uint useIndex = indexList[index];

    for(int i = 0; i < sphereCount; i++)
    {
        Sphere s = spheres[i];
        float3 pos = positions[useIndex];
        float3 dir = pos - s.center;
        float dist = length(dir);

        // 近ければ近いほど強く押し出される
        if (dist < s.radius)
        {
            float3 n = normalize(dir);
            pos += n * (s.radius - dist + ignoreSpeed);
            positions[useIndex] = pos;
        }
    }

    return 0;
}
