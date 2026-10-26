cbuffer ParticleCount : register(b0)
{
    uint particleCount;
};

cbuffer TextureConfig : register(b1)
{
    uint2 textureSize;
};

StructuredBuffer<uint> indexList : register(t0);

Texture2D<float4> texture : register(t1);
SamplerState gSampler : register(s0);

RWStructuredBuffer<float3> tmp : register(u0);
RWStructuredBuffer<float> scales : register(u1);
RWStructuredBuffer<float16_t4> color : register(u2);
RWStructuredBuffer<float3> positions : register(u3);

[numthreads(128, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    int index = DTid.x;
    if (index >= particleCount)
    {
        return;
    }
    
    // uvを求める
    float2 uv = float2(
    (index % textureSize.x) / (float) textureSize.x,
    (index / textureSize.x) / (float) textureSize.y);
    
    float4 texColor = texture.SampleLevel(gSampler, uv, 0);
    
    // 透明なところなら除外
    if (texColor.a < 0.1f)
    {
        return;
    }
    
    uint localIndex = indexList[index];
    
    positions[index] = (float3(uv.x, 1.0f - uv.y, 0.0f) + float3(-0.5f, -0.5f, 0.0f)) * float3(textureSize.x * 0.1, textureSize.y * 0.1, 0.0f);
    scales[localIndex] = 0.11f;
    color[localIndex] = float16_t4(texColor.r, texColor.g, texColor.b, texColor.a);
}
