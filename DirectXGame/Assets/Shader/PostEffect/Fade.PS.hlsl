#include "PostEffect.hlsli"

cbuffer FadeParameters : register(b1)
{
    float4 fadeColor; // フェードカラー
    float t; // フェードの進行度 (0.0 = no fade, 1.0 = full fade)
}

cbuffer TextureIndex : register(b0)
{
    int textureIndex; // 使用するテクスチャのインデックス
};

Texture2D<float4> gTexture[] : register(t8);
SamplerState gSampler : register(s0);

PixelShaderOutput main(PixelShaderInput input) {
    PixelShaderOutput output;
    // テクスチャから色を取得
    float4 origin = gTexture[textureIndex].Sample(gSampler, input.texcoord);
    
    float factT = t * fadeColor.a; // フェードの進行度にアルファ値を掛ける
    output.color.rgb = lerp(origin.rgb, fadeColor.rgb, factT);
    output.color.a = 1.0f;
    
    return output;
}
