struct PSInput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD1;
};

struct PSOutput
{
    float4 color : SV_TARGET;
};

cbuffer TextureIndex : register(b0)
{
    int textureIndex;
}

cbuffer FontColor : register(b1)
{
    float4 fontColor;
}

Texture2D gTexture[] : register(t8);
SamplerState gSampler : register(s0);

PSOutput main(PSInput input)
{
    PSOutput output;
    float fontAlpha = gTexture[textureIndex].Sample(gSampler, input.texcoord).a;
    output.color = fontColor;
    output.color.a = fontColor.a * fontAlpha;
    if(output.color.a < 0.1)
    {
        discard;
    }
    return output;
}
