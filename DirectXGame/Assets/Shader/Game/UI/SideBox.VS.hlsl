struct VSInput
{
    float3 position : POSITION0;
    float2 texcoord : TEXCOORD0;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
};

cbuffer MatrixData : register(b0)
{
    float4x4 wvp;
};

VSOutput main(VSInput input, uint instance : SV_InstanceID)
{
    VSOutput output;
    output.position = mul(float4(input.position, 1.0f), wvp);
    output.texCoord = input.texcoord;
    return output;
}
