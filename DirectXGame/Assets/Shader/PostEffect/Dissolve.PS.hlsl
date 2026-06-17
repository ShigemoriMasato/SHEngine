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
    uint noiseTextureIndex;
    uint transitionTextureIndex;
    float threshold; //0.0〜1.0の範囲で、ノイズテクスチャのどの明るさを境界とするか
    float edgeThreshold; // エッジのしきい値を調整する定数
    float3 edgeColor; // エッジの色を設定
};

float smoothStep(float edge0, float edge1, float x)
{
    float t = saturate((x - edge0) / (edge1 - edge0));
    return t * t * (3.0f - 2.0f * t);
}

float4 main(PixelShaderInput input) : SV_TARGET
{
    float4 output = float4(0, 0, 0, 1);
    
    float4 noiseColor = textures[noiseTextureIndex].Sample(gSampler, input.texcoord);
    float4 transitionColor = textures[transitionTextureIndex].Sample(gSampler, input.texcoord);
    float4 mainColor = textures[textureIndex].Sample(gSampler, input.texcoord);
    
    if(noiseColor.r > threshold)
    {
        output = mainColor;
    }
    else
    {
        return transitionColor;
    }
    
    float edgeFactor = 1.0f - smoothStep(threshold, threshold + edgeThreshold, noiseColor.r);
   
    output.rgb += edgeFactor * edgeColor; // エッジの色を加算することで、エッジを強調する
    
    return output;
}