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
    float2 kCenter; //中心点。ここを基準に放射状にブラーがかかる
    float kBlurWidth; //ぼかしの幅。
    int kNumSamples; //サンプリング数。多いほど滑らかだが重い
};

float4 main(PixelShaderInput input) : SV_TARGET
{
    //中心から現在のuvに対しての方向を計算。
    //普段方向といえば、単位ベクトルだが、ここではあえて正規化せず、遠いほどより遠くをサンプリングする
    float2 direction = input.texcoord - kCenter;
    float3 outputColor = float3(0.0f, 0.0f, 0.0f);
    for (int sampleIndex = 0; sampleIndex < kNumSamples; ++sampleIndex)
    {
        //現在のuvからさきほど計算した方向にサンプリング点を進めながらサンプリングしていく
        float2 texcoord = input.texcoord + direction * kBlurWidth * float(sampleIndex);
        outputColor.rgb += textures[textureIndex].Sample(gSampler, texcoord).rgb;
    }

    //平均化する
    outputColor.rgb *= rcp(float(kNumSamples));

    float4 output;
    output.rgb = outputColor;
    output.a = 1.0f;
    return output;
}
