#include "PostEffect.hlsli"

Texture2D textures[] : register(t8);
SamplerState gSampler : register(s0);

cbuffer TextureIndex : register(b0)
{
    int textureIndex;
}

PixelShaderOutput main(PixelShaderInput input)
{
    PixelShaderOutput output;
    output.color = textures[textureIndex].Sample(gSampler, input.texcoord);
    if (output.color.a < 0.01f)
    {
        discard;
    }
    return output;
}
