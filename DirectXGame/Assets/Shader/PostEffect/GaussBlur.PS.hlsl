#include "PostEffect.hlsli"

static const float PI = 3.14159265358979323846;

cbuffer TextureIndex : register(b0)
{
    int textureIndex; // 使用するテクスチャのインデックス
};

Texture2D<float4> textures[] : register(t8);
SamplerState gSampler : register(s0);
 
cbuffer Data : register(b1)
{
    uint clientKernel; // ブラーの幅
    float sigma; // ガウス関数の標準偏差
};

float gauss(float x, float y, float sigma)
{
    float exponent = -(x * x + y * y) / (2.0 * sigma * sigma);
    float denominator = 2.0 * PI * sigma * sigma;
    return exp(exponent) / denominator;
}

float4 main(PixelShaderInput input) : SV_TARGET
{
    float4 output = float4(0, 0, 0, 0);
    
    int kernelSize = clamp(clientKernel, 0, 16);
    
    uint2 buff;
    textures[textureIndex].GetDimensions(buff.x, buff.y);
    float width = (float) buff.x;
    float height = (float) buff.y;
    
    float midW = kernelSize / 2;
    float midH = kernelSize / 2;
    
    int2 offset[16][16];
    for (int i = 0; i < kernelSize; ++i)
    {
        for (int j = 0; j < kernelSize; ++j)
        {
            offset[i][j] = int2(j - midW, i - midH);
        }
    }
    
    float weight = 0.0f;
    float kernel[16][16];
    for (int i = 0; i < kernelSize; ++i)
    {
        for (int j = 0; j < kernelSize; ++j)
        {
            kernel[i][j] = gauss(offset[i][j].x, offset[i][j].y, sigma);
            weight += kernel[i][j];
        }
    }

    float baseX = input.texcoord.x * width;
    float baseY = input.texcoord.y * height;
    
    for (int i = 0; i < kernelSize; i++)
    {
        for (int j = 0; j < kernelSize; j++)
        {
            float x = baseX + offset[i][j].x;
            float y = baseY + offset[i][j].y;
            
            // テクスチャの範囲外をサンプリングしないようにする
            x = clamp(x, 0, width - 1);
            y = clamp(y, 0, height - 1);
            
            output += textures[textureIndex].Sample(gSampler, float2(x / (float) width, y / (float) height)) * kernel[i][j];
        }
    }
    
    // ブラーの平均を取る
    output *= rcp(weight);
    return output;
}