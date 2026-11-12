#include "PostEffect.hlsli"

Texture2D<float4> textures[] : register(t8);
SamplerState gSampler : register(s0);

cbuffer TextureIndex : register(b0)
{
    uint textureIndex;
};

cbuffer BlurData : register(b1)
{
    uint sampleCount;
    uint2 textureSize;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
    float4 color = float4(0, 0, 0, 1.0f);
    float offset = 1.0 / textureSize.y; // 垂直方向のオフセット
    float scale = 1.0 / sampleCount; // サンプル数で割ることで平均化
    float2 baseUV = input.texcoord - float2(0.0f, offset * sampleCount / 2);
    
    for (uint i = 0; i < sampleCount; ++i)
    {
        float2 uv = baseUV + float2(0.0f, i * offset);
        color.rgb += textures[textureIndex].Sample(gSampler, uv).rgb * scale;
    }
    
    return color;
}
