cbuffer Counter : register(b0)
{
    uint particleCount;
};

cbuffer DeltaTime : register(b1)
{
    float deltatime;
};

cbuffer SpeedConfig : register(b2)
{
    float threshold;
    float attenuation;
};

cbuffer floor : register(b2)
{
    float floorY;
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
    
    velocities[index] += forces[index] * deltatime;
    
    float3 velocity = velocities[index];
    float speed = length(velocity);
    if (speed > threshold)
    {
        float diff = speed - threshold;
        // threadshold を超えただけ減衰が強くなる 
        float attenuationFactor = 1.0 - (diff / speed) * attenuation;
        velocity = velocity * attenuationFactor;
    }
    
    velocities[index] = velocity;
    positions[index] += velocities[index] * deltatime;
    
    if (positions[index].y < floorY)
    {
        positions[index].y = floorY;
        velocities[index].y = 0;
    }
    
    forces[index] = float3(0, 0, 0);
}
