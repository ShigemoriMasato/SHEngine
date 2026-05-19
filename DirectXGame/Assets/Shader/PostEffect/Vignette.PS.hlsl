#include "PostEffect.hlsli"

cbuffer TextureIndex : register(b0)
{
    int textureIndex; // 使用するテクスチャのインデックス
};

Texture2D<float4> textures[] : register(t8);
SamplerState gSampler : register(s0);
 
cbuffer Data : register(b1)
{
    float4 color; // 色
    float intensity; // 減光の強さ (0.0 ~ 1.0)
    float radius; // 明るさが保たれる半径 (0.0 ~ 1.0)
    float softness; // グラデーションの滑らかさ (0.0 ~ 1.0)
};

float4 main(PixelShaderInput input) : SV_TARGET
{
    float2 uv = input.texcoord;

    // 中心(0.5, 0.5)からの距離
    float2 center = float2(0.5, 0.5);
    float dist = distance(uv, center);

    // vignette係数計算
    float vignette = smoothstep(radius, radius - softness, dist);

    // 強さ適用（反転して暗くする）
    vignette = lerp(1.0, vignette, intensity);

    float4 texColor = textures[textureIndex].Sample(gSampler, uv);

    texColor = lerp(color, texColor, vignette);

    return texColor;
}