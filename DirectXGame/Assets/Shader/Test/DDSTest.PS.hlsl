
struct PSInput
{
    float4 position : SV_POSITION;
    float3 texCoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

struct PSOutput
{
    float4 color : SV_TARGET;
};

TextureCube<float4> textures : register(t0);
SamplerState gSampler : register(s0);

PSOutput main(PSInput input)
{
    PSOutput output;
    output.color = textures.Sample(gSampler, input.texCoord);
    return output;
}
