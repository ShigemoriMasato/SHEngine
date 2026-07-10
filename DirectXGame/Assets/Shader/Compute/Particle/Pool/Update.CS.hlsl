cbuffer ParticleNum : register(b0)
{
    uint maxNum;
};

RWStructuredBuffer<float32_t3> outputPositions : register(u0);
RWStructuredBuffer<float16_t4> outputColors : register(u1);

StructuredBuffer<float32_t3> positions : register(t0);
StructuredBuffer<float16_t4> colors : register(t1);

[numthreads(1024, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;
    if(index >= maxNum)
        return;

    outputPositions[index] = positions[index];
    outputColors[index] = colors[index];
}