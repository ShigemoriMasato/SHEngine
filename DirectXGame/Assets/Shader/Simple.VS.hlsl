struct VSInput
{
    float4 position : POSITION0;
    float2 texCoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

cbuffer MatrixBuffer : register(b0)
{
    float4x4 wvp;
}

VSOutput main(VSInput input)
{
    VSOutput output;
    output.position = mul(input.position, wvp);
    output.texCoord = input.texCoord;
    output.normal = input.normal;
	return output;
}