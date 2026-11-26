cbuffer ParticleCount : register(b0)
{
    uint particleCount;
};

cbuffer Config : register(b1)
{
    uint emitOffset;
    uint emitCount;
    uint seed;
    float deltaTime;

    float spread;
    float3 emitPos;
    float speed;
    float3 emitDir;

    float4 color;
    float scale;
};

StructuredBuffer<uint> indexList : register(t0);

RWStructuredBuffer<float3> positions : register(u0);
RWStructuredBuffer<float3> rotations : register(u1);
RWStructuredBuffer<float> scales : register(u2);
RWStructuredBuffer<float16_t4> color : register(u3);
RWStructuredBuffer<float3> velocity : register(u4);

void Hash(inout uint x)
{
    x ^= x >> 16;
    x *= 0x7feb352d;
    x ^= x >> 15;
    x *= 0x846ca68b;
    x ^= x >> 16;
}

float randf(inout uint state)
{
    float value = Hash(state) / 4294967296.0;
    return value;
}

[numthreads(128, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    int index = DTid.x;
    if (index >= emitCount)
    {
        return;
    }
    
    uint useIndex = emitOffset + index;
    uint state = seed + useIndex;

    float3 n = normalize(emitDir);

    float cosMax = cos(saturate(spread) * PI);

    float cosTheta = lerp(1.0f, cosMax, randf(state));
    float sinTheta = sqrt(saturate(1.0f - cosTheta * cosTheta));
    float phi      = 2.0f * PI * randf(state);

    float sinPhi, cosPhi;
    sincos(phi, sinPhi, cosPhi);

    float  s  = (n.z >= 0.0f) ? 1.0f : -1.0f;
    float  a  = -1.0f / (s + n.z);
    float  b  = n.x * n.y * a;
    float3 t  = float3(1.0f + s * n.x * n.x * a, s * b, -s * n.x);
    float3 bt = float3(b, s + n.y * n.y * a, -n.y);

    float3 vel = normalize(t  * (cosPhi * sinTheta) + bt * (sinPhi * sinTheta) + n  * cosTheta) * speed;

    velocity[useIndex] = vel;
    positions[useIndex] = emitPos + vel * deltaTime * index / emitCount;
    rotations[useIndex] = float3(0.0f, 0.0f, 0.0f);
    scales[useIndex] = scale;
    color[useIndex] = float16_t4(color.r, color.g, color.b, color.a);
}