cbuffer ParticleCount : register(b0)
{
    uint particleCount;
};

cbuffer Config : register(b1)
{
    float floor;
    float3 gravity;

    float cor;      // 反発係数
    float attenuation;

    float deltatime;
}

StructuredBuffer<uint> indexList : register(t0);

RWStructuredBuffer<float3> positions : register(u0);
RWStructuredBuffer<float3> rotations : register(u1);
RWStructuredBuffer<float> scales : register(u2);
RWStructuredBuffer<float16_t4> color : register(u3);
RWStructuredBuffer<float3> velocity : register(u4);

[numthreads(128, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    int index = DTid.x;
    if (index >= particleCount)
    {
        return;
    }
    
    uint localIndex = indexList[index];
    
    if (scales[localIndex] < 1e-6f)
    {
        return;
    }
    
    float3 pos = positions[localIndex];
    float3 vel = velocity[localIndex];

    vel *= attenuation;
    vel += gravity * deltatime;

    pos += vel * deltatime;

    if(pos.y < floor)
    {
        pos.y = floor;
        vel.y *= -cor;
    }

    positions[localIndex] = pos;
    velocity[localIndex] = vel;
}
