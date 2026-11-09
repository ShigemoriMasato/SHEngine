// Shared particle-pool contract: process index i through indexList[i].
cbuffer ParticleCount : register(b0)
{
    uint particleCount;
};
StructuredBuffer<uint> indexList : register(t0);
RWStructuredBuffer<float3> position : register(u0);
RWStructuredBuffer<float> scales : register(u1);
RWStructuredBuffer<float16_t4> color : register(u2);

cbuffer BeamConfig : register(b1)
{
    float3 startPoint;
    float emissionRate;

    float3 endPoint;
    float innerRadius;

    float4 colorMultiplier;

    float outerRadius;
    float particleRadius;
    uint emissionCount;
    uint emissionCursor;

    uint seed;
    float particleSpeed;
    float deltaTime;
    float convergenceRate;
    float randomness;
    float3 padding;
};
Texture2D<float4> colorTexture : register(t1);
SamplerState colorSampler : register(s0);

static const float PI = 3.14159265358979323846f;
uint Hash(uint x)
{
    x ^= x >> 16;
    x *= 0x7feb352d;
    x ^= x >> 15;
    x *= 0x846ca68b;
    x ^= x >> 16;
    return x;
}
float Random01(inout uint s)
{
    s = Hash(s);
    return (s >> 8) * (1.0f / 16777216.0f);
}

[numthreads(128, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint i = DTid.x;
    if (i >= particleCount)
        return;
    
    float3 axis = endPoint - startPoint;
    float axisLength = length(axis);
    float3 n = axisLength > 1.0e-6f ? axis / axisLength : float3(0, 0, 0);
    uint birthCount = min(emissionCount, particleCount);

    // Advance existing particles toward the endpoint. Skip slots that this
    // dispatch is about to overwrite so each pool element has a single writer.
    uint distanceFromCursor = (i + particleCount - emissionCursor) % particleCount;
    bool willBeReused = distanceFromCursor < birthCount;
    uint poolIndex = indexList[i];
    if (!willBeReused && scales[poolIndex] > 0.0f && axisLength > 1.0e-6f)
    {
        float currentDistance = dot(position[poolIndex] - startPoint, n);
        float nextDistance = currentDistance + max(particleSpeed, 0.0f) * max(deltaTime, 0.0f);
        if (nextDistance >= axisLength)
            scales[poolIndex] = 0.0f;
        else
        {
            float currentProgress = saturate(currentDistance / axisLength);
            float nextProgress = saturate(nextDistance / axisLength);
            float currentRadiusScale = max(0.0f, 1.0f - convergenceRate * currentProgress);
            float nextRadiusScale = max(0.0f, 1.0f - convergenceRate * nextProgress);
            float3 radialOffset = position[poolIndex] - (startPoint + n * currentDistance);
            radialOffset = currentRadiusScale > 1.0e-5f
                ? radialOffset * (nextRadiusScale / currentRadiusScale)
                : float3(0.0f, 0.0f, 0.0f);
            position[poolIndex] = startPoint + n * nextDistance + radialOffset;
        }
    }

    
    if (i >= birthCount || particleCount == 0 || axisLength <= 1.0e-6f)
        return;
    
    uint relativeSlot = (emissionCursor + i) % particleCount;
    uint dst = indexList[relativeSlot];
    uint rng = Hash(seed ^ relativeSlot);
    // Every new particle starts at the beam origin and then flows down the axis.
    float theta = Random01(rng) * 2.0f * PI;
    float radial = sqrt(lerp(innerRadius * innerRadius, outerRadius * outerRadius, Random01(rng)));
    float3 jitter = (float3(Random01(rng), Random01(rng), Random01(rng)) * 2.0f - 1.0f) * max(randomness, 0.0f);
    float3 helper = abs(n.y) < 0.95f ? float3(0, 1, 0) : float3(1, 0, 0);
    float3 u = normalize(cross(n, helper));
    float3 v = cross(n, u);
    
    position[dst] = startPoint + radial * (cos(theta) * u + sin(theta) * v) + jitter;
    scales[dst] = max(particleRadius, 0.0f);
    
    float4 textureColor = colorTexture.SampleLevel(colorSampler, float2(0.0f, theta / (2.0f * PI)), 0);
    color[dst] = float16_t4(textureColor * colorMultiplier);
}
