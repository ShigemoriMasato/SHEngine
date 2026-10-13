cbuffer Counter : register(b0)
{
    uint particleCount;
};

cbuffer Scale : register(b1)
{
    float scale;
};

cbuffer Deltatime : register(b2)
{
    float deltatime;
}

RWStructuredBuffer<float3> positions : register(u0);
RWStructuredBuffer<float3> forces : register(u1);
RWStructuredBuffer<float3> velocities : register(u2);

float3 RemoveForce(float3 a, float3 b)
{
    float doted = dot(a, b);
    float lengthSq = dot(b, b);
    
    if (lengthSq == 0.0f)
    {
        return a;
    }
    
    return a - b * (doted / lengthSq);
}

[numthreads(128, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;
    if (index >= particleCount)
    {
        return;
    }
    
    float collisionDistance = scale * 0.5f;
    float3 position = positions[index];
    
    for (int i = 0; i < particleCount; ++i)
    {
        if (i == index)
        {
            continue;
        }
        
        float3 diff = positions[i] - position;
        float dist = length(diff);
        
        if (dist < collisionDistance)
        {
            float3 normalizedDiff = normalize(diff);
            position -= normalizedDiff * (collisionDistance - dist) * 1.2f;
            velocities[index] = RemoveForce(velocities[index], normalizedDiff);
        }
    }
    
    positions[index] = position + velocities[index] * deltatime;
}
