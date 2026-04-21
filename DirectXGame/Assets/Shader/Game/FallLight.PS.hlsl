
struct PSInput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
    float3 normal : NORMAL0;
    float4 color : COLOR0;
};

struct PSOutput
{
    float4 color : SV_TARGET;
};

cbuffer TextureIndexBuffer : register(b0)
{
    uint textureIndex;
}

Texture2D<float4> gTexture[] : register(t8);
SamplerState gSampler : register(s0);

PSOutput main(PSInput input)
{
    PSOutput output;
    output.color = gTexture[textureIndex].Sample(gSampler, input.texCoord) * input.color;
    return output;
}