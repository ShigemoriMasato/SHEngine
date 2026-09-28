cbuffer ParticleCount : register(b0)
{
    int particleCount;
}

cbuffer ParticleInfo : register(b1)
{
    float deltaTime;
    float mass;
}

StructuredBuffer<uint> isAlive : register(t1);

RWStructuredBuffer<float3> position : register(u0);
RWStructuredBuffer<float3> velocity : register(u1);
RWStructuredBuffer<float3> force : register(u2);

[numthreads(128, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;
    
    // パーティクルの最大数を超えていたら終了
    if (index >= particleCount)
        return;
    
    // パーティクルが生きていなければ終了
    if (isAlive[index] == 0)
        return;
    
    velocity[index] += force[index] / mass * deltaTime;
    position[index] += velocity[index] * deltaTime;
    
    force[index] = float3(0.0f, 0.0f, 0.0f);
}
