cbuffer Counter : register(b0)
{
    uint particleCount;
};

cbuffer Scale : register(b1)
{
    float scale;
};

RWStructuredBuffer<float3> positions : register(u0);
RWStructuredBuffer<float3> forces : register(u1);
RWStructuredBuffer<float3> velocities : register(u2);

[numthreads(128, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;
    if (index >= particleCount)
    {
        return;
    }
    
    float collisionDistance = scale * 2.f;
    
    for (int i = 0; i < particleCount; ++i)
    {
        if (i == index)
        {
            continue;
        }
        
        float3 diff = positions[index] - positions[i];
        float len = length(diff);
        
        if (len < collisionDistance)
        {
            float3 dir = normalize(diff);
            
            float hitStrength = length(velocities[index] - velocities[i]);
            forces[index] += dir * hitStrength;
            //positions[index] += dir * (collisionDistance - len);
        }
    }
}
