cbuffer CSData : register(b0)
{
    float3 color;
    uint vertexNum;
};
cbuffer ID : register(b1)
{
    uint id;
};

RWStructuredBuffer<float32_t3> positions : register(u0);
RWStructuredBuffer<float32_t4> colors : register(u1);
RWStructuredBuffer<float32_t3> vertices : register(u2);
RWStructuredBuffer<uint32_t> particleCount : register(u3);

Texture2D<float4> textures[] : register(t8);
SamplerState gSampler : register(s0);

uint Hash(uint x)
{
    x ^= x >> 16;
    x *= 0x7feb352d;
    x ^= x >> 15;
    x *= 0x846ca68b;
    x ^= x >> 16;
    return x;
}

float Rand(inout uint state)
{
    state = Hash(state);
    return state / 4294967296.0;
}

[numthreads(64, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;
    
    if (index >= vertexNum)
    {
        return;
    }
    
    positions[index] = vertices[index];
    colors[index] = float4(color, 1.0);
    InterlockedAdd(particleCount[0], 1, particleCount[0]);
}
