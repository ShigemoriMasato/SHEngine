cbuffer ParticleCount : register(b0)
{
    uint particleCount;
};

cbuffer HeightScale : register(b1)
{
    float heightScale;
};

StructuredBuffer<uint> indexList : register(t0);
StructuredBuffer<float2> uvs : register(t1);

Texture2D<float4> colorTexture : register(t2);
Texture2D<float4> heightTexture : register(t3);
SamplerState gSampler : register(s0);

RWStructuredBuffer<float3> position : register(u0);
RWStructuredBuffer<float> scales : register(u1);
RWStructuredBuffer<float16_t4> color : register(u2);

[numthreads(128, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    int index = DTid.x;
    if (index >= particleCount)
    {
        return;
    }
    
    uint localIndex = indexList[index];
    
    if (scales[localIndex] < 1e-6f)
    {
        return;
    }
    
    float4 texColor = colorTexture.SampleLevel(gSampler, uvs[localIndex], 0);
    float4 heightColor = heightTexture.SampleLevel(gSampler, uvs[localIndex], 0);
    position[localIndex].y = heightColor.r * heightScale;
    color[localIndex] = float16_t4(texColor.r, texColor.g, texColor.b, texColor.a);
}
