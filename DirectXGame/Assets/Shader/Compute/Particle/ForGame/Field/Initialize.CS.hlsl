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
    // 敷き詰める場合は0.1f
    float scale;
}

StructuredBuffer<uint> indexList : register(t0);

RWStructuredBuffer<float3> positions : register(u0);
RWStructuredBuffer<float> scales : register(u1);
RWStructuredBuffer<float16_t4> color : register(u2);
RWStructuredBuffer<float2> uvs : register(u3);

float4 ColorGradation(float t)
{
    t = saturate(t);
    if (t < 1.0 / 3.0)
    {
        // 赤 → 緑
        float u = t * 3.0;
        return float4(1.0 - u, u, 0.0, 1.0);
    }
    else if (t < 2.0 / 3.0)
    {
        // 緑 → 青
        float u = (t - 1.0 / 3.0) * 3.0;
        return float4(0.0, 1.0 - u, u, 1.0);
    }
    else
    {
        // 青 → 白
        float u = (t - 2.0 / 3.0) * 3.0;
        return float4(u, u, 1.0, 1.0);
    }
}

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
    
    uint localIndex = indexList[index];
    
    uvs[localIndex] = uv;
    positions[localIndex] = (float3(uv.x, 0.0f, 1 - uv.y) + float3(-0.5f, 0.0f, -0.5f)) * float3(textureSize.x * 0.1, 1.0f, textureSize.y * 0.1);
    scales[localIndex] = scale;
    
    // debug
    float distRatio = saturate(length(positions[localIndex].xz) / (60.f));
    color[localIndex] = float16_t4(ColorGradation(distRatio)) * float16_t4(0.3f, 0.3f, 0.3f, 1.0f);
}
