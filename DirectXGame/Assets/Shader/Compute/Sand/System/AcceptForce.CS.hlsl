cbuffer Counter : register(b0)
{
    uint particleCount;
};

cbuffer DeltaTime : register(b1)
{
    float deltatime;
};

cbuffer Config : register(b2)
{
    float threshold;
    float attenuation;
    
    float floorY;
};

cbuffer Gravity : register(b3)
{
    float3 gravity;
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
    
    float3 position = positions[index];
    float3 velocity = velocities[index];
    velocity += forces[index] * deltatime;
    
    float speed = length(velocity);
    if (speed > threshold)
    {
        float diff = speed - threshold;
        // threadshold を超えただけ減衰が強くなる 
        float attenuationFactor = 1.0 - (diff / speed) * attenuation;
        velocity = velocity * attenuationFactor;
    }
    
    velocity = velocity;
    position += velocity * deltatime;
    
    if (position.y < floorY)
    {
        position.y = floorY;
        velocity.y *= 0.f;
        velocity *= 0.75f;
        forces[index] = float3(0, 0, 0);
    }
    else
    {
        forces[index] = gravity;
    }
    
    positions[index] = position;
    velocities[index] = velocity;
}
