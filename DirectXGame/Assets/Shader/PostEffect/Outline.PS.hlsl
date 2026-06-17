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
    float4 color;
    uint edgeTextureIndex;
    float strength;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
    float4 output = float4(0, 0, 0, 1);
    output = textures[textureIndex].Sample(gSampler, input.texcoord);
    float4 edgeColor = textures[edgeTextureIndex].Sample(gSampler, input.texcoord);
    
    if (edgeColor.r > (1.0f - strength))
    {
        output = color;
    }
    
    return output;
}