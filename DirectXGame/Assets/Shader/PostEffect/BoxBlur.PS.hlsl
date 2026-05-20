#include "PostEffect.hlsli"

cbuffer TextureIndex : register(b0)
{
    int textureIndex; // 使用するテクスチャのインデックス
};

Texture2D<float4> textures[] : register(t8);
SamplerState gSampler : register(s0);
 
cbuffer Data : register(b1)
{
    uint blurWidth; // ブラーの幅
    uint blurHeight; // ブラーの高さ
};

float4 main(PixelShaderInput input) : SV_TARGET
{
    float4 output = float4(0, 0, 0, 0);
    
    uint2 buff;
    textures[textureIndex].GetDimensions(buff.x, buff.y);
    float width = (float) buff.x;
    float height = (float) buff.y;
    
    float midW = blurWidth / 2;
    float midH = blurHeight / 2;
    
    int2 centerPixel = int2(int(input.texcoord.x * width), int(input.texcoord.y * height));
    
    for (int i = 0; i < blurHeight; i++)
    {
        for (int j = 0; j < blurWidth; j++)
        {
            float x = input.texcoord.x * width + j - midW;
            float y = input.texcoord.y * height + i - midH;
            
            // テクスチャの範囲外をサンプリングしないようにする
            x = clamp(x, 0, width - 1);
            y = clamp(y, 0, height - 1);
            
            output += textures[textureIndex].Sample(gSampler, float2(x / (float)width, y / (float)height));
        }
    }
    
    // ブラーの平均を取る
    output /= (blurWidth * blurHeight);
    return output;
}