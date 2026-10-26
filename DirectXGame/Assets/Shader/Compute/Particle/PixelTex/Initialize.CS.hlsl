cbuffer ParticleCount : register(b0)
{
    uint particleCount;
};

cbuffer TextureConfig : register(b1)
{
    uint2 textureSize;
};

cbuffer Scale : register(b2)
{
    // 敷き詰める場合は0.11f
    float scale;
}

StructuredBuffer<uint> indexList : register(t0);

Texture2D<float4> texture : register(t1);
SamplerState gSampler : register(s0);

RWStructuredBuffer<float3> positions : register(u0);
RWStructuredBuffer<float> scales : register(u1);
RWStructuredBuffer<float16_t4> color : register(u2);
RWStructuredBuffer<float2> uvs : register(u3);

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
    
    uvs[localIndex] = uv;
    positions[localIndex] = (float3(uv.x, 0.0f, 1 - uv.y) + float3(-0.5f, 0.0f, -0.5f)) * float3(textureSize.x * 0.1, 1.0f, textureSize.y * 0.1);
    scales[localIndex] = scale;
    color[localIndex] = float16_t4(texColor.r, texColor.g, texColor.b, texColor.a);
}
