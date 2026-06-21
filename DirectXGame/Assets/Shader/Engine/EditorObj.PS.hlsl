
struct VSOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
    float4 colorID : COLOR0;
};

struct PSOutput
{
    float4 color : SV_TARGET0;
    float4 idColor : SV_TARGET1;
};

cbuffer TextureIndex : register(b0)
{
    int textureIndex;
}

Texture2D textures[] : register(t8);
SamplerState gSampler : register(s0);

PSOutput main(VSOutput input)
{
    PSOutput output;
    output.color = textures[textureIndex].Sample(gSampler, input.texcoord);
    output.idColor = input.colorID;
    return output;
}
