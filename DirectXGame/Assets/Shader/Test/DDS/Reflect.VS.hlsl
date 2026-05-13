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
    float3 worldPos : POSITION0;
};

cbuffer VSData : register(b0)
{
    float4x4 world;
    float4x4 wvp;
};

VSOutput main(VSInput input)
{
    VSOutput output;
    output.position = mul(input.position, wvp);
    output.texCoord = input.texCoord;
    output.normal = mul(float4(input.normal, 1.0f), world).xyz;
    output.worldPos = mul(input.position, world).xyz;
	return output;
}