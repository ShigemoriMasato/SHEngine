#include "PostEffect.hlsli"

Texture2D<float4> textures[] : register(t8);
SamplerState gSampler : register(s0);

cbuffer SourceTextureIndex : register(b0)
{
    uint srcIndex;
};

cbuffer CompositeTextureIndex : register(b1)
{
    uint compositeIndex;
};

cbuffer BlurData : register(b2)
{
    uint4 pad;
    float strength = 1.0f;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
    float4 src = textures[srcIndex].Sample(gSampler, input.texcoord);
    float4 cmp = textures[compositeIndex].Sample(gSampler, input.texcoord);
    
    float4 output;
    output = src + cmp * strength;
    
    return output;
}
