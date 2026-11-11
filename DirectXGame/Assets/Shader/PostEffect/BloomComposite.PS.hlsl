#include "PostEffect.hlsli"

Texture2D<float4> textures[] : register(t8);
SamplerState gSampler : register(s0);

cbuffer BlurTextureIndex : register(b0)
{
    uint textureIndex;
};

cbuffer CompositeData : register(b1)
{
    uint clientKernel;
    float sigma;
    float strength;
    int maskTexture_;
    
    uint iterations;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
    float offset = 0.0f;
    float scale = 0.5f;
    float4 color = float4(0.0f, 0.0f, 0.0f, 1.0f);
    
    for (int i = 0; i < iterations; ++i)
    {
        float2 uv = float2(2.f, 1.f) * input.texcoord * scale + float2(0.0f, offset);
        float4 textureColor = textures[textureIndex].Sample(gSampler, uv);
        color.rgb += textureColor.rgb / iterations;
        
        offset += scale;
        scale *= 0.5f;
    }

    return color;
}
