#include "PostEffect.hlsli"

cbuffer TextureIndex : register(b0)
{
    int textureIndex; // 使用するテクスチャのインデックス
};

Texture2D<float4> textures[] : register(t8);
SamplerState gSampler : register(s0);
 
//PostEffectに使用する値を格納する定数バッファ。512バイトまで格納可能
cbuffer Data : register(b1)
{
    uint targetTextureIndex;
};

float Luminance(float3 color)
{
    return dot(color, float3(0.2125, 0.7154, 0.0721));
}

float4 main(PixelShaderInput input) : SV_TARGET
{
    float4 output = float4(0, 0, 0, 1);
    
    static const float kPrewittHorizontalKernel[3][3] = {
        { 1.0f / 6.0f, 0, -1.0f / 6.0f },
        { 1.0f / 6.0f, 0, -1.0f / 6.0f },
        { 1.0f / 6.0f, 0, -1.0f / 6.0f }
    };
    
    static const float kPrewittVerticalKernel[3][3] = {
        { -1.0f / 6.0f, -1.0f / 6.0f, -1.0f / 6.0f },
        { 0, 0, 0 },
        { 1.0f / 6.0f, 1.0f / 6.0f, 1.0f / 6.0f }
    };
    
    static const float2 kOffsets[9] = {
        float2(-1, -1), float2(0, -1), float2(1, -1),
        float2(-1, 0),  float2(0, 0),  float2(1, 0),
        float2(-1, 1),  float2(0, 1),  float2(1, 1)
    };
    
    uint width, height;
    textures[targetTextureIndex].GetDimensions(width, height);
    float2 uvStepSize = float2(rcp(float(width)), rcp(float(height)));
    
    float2 difference = float2(0, 0);
    for (int x = 0; x < 3; ++x)
    {
        for (int y = 0; y < 3; ++y)
        {
            float2 offset = kOffsets[y * 3 + x] * uvStepSize;
            float4 sampleColor = textures[targetTextureIndex].Sample(gSampler, input.texcoord + offset);
            float luminance = Luminance(sampleColor.rgb);
            difference.x += luminance * kPrewittHorizontalKernel[y][x];
            difference.y += luminance * kPrewittVerticalKernel[y][x];
        }
    }
    
    float weight = length(difference);
    weight = saturate(pow(weight, 0.1));
    output.r = weight;
    output.a = 1.0f;
    return output;
}