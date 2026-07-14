struct Particle
{
    float3 position;
    float lifetime;
    float3 scale;
    float currentTime;
    float4 color;
    float3 velocity;
};

RWStructuredBuffer<Particle> particleList : register(u0);

static const uint maxCount = 1024;

[numthreads(1024, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint id = DTid.x;
    if(id >= maxCount)
        return;

    particleList[id] = (Particle)0;
    particleList[id].position = float3(-15.f, 0.0f, 0.0f);
    particleList[id].scale = float3(0.5f, 0.5f, 0.5f);
    particleList[id].color = float4(1,1,1,1);
}