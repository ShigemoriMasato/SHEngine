#include "PostEffect.hlsli"

cbuffer Data : register(b1)
{
    float intensity; // グレースケールの強度（0.0〜1.0）
}

cbuffer TextureIndex : register(b0)
{
    int textureIndex; // 使用するテクスチャのインデックス
};

Texture2D<float4> textures[] : register(t8);
SamplerState gSampler : register(s0);

PixelShaderOutput main(PixelShaderInput input)
{
    PixelShaderOutput output;
    
    float4 texColor = textures[textureIndex].Sample(gSampler, input.texcoord);
    //色の平均をとる
    float luminance = dot(texColor.rgb, float3(0.2125, 0.7154, 0.0721));
    float3 grayscale = float3(luminance, luminance, luminance);
    output.color = float4(lerp(texColor.rgb, grayscale, intensity), texColor.a);
    
    return output;
}
