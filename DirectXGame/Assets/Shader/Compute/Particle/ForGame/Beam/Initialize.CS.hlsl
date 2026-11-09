// Initialize exactly the indices assigned to this process by the particle pool.
cbuffer ParticleCount : register(b0) { uint particleCount; };
StructuredBuffer<uint> indexList : register(t0);
RWStructuredBuffer<float3> position : register(u0);
RWStructuredBuffer<float> scales : register(u1);
RWStructuredBuffer<float16_t4> color : register(u2);
[numthreads(128, 1, 1)]
void main(uint3 id : SV_DispatchThreadID) {
    if (id.x >= particleCount) return;
    uint poolIndex = indexList[id.x];
    position[poolIndex] = 0.0f;
    scales[poolIndex] = 0.0f;
    color[poolIndex] = float16_t4(1, 1, 1, 1);
}
