struct VSInput
{
    float4 position : POSITION0;
    float2 texCoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float3 texCoord : TEXCOORD0;
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
    // -0.5 ~ 0.5の立方体が来る想定なので、二倍して-1~1の範囲にする
    output.texCoord = input.position.xyz * 2;
    output.normal = input.normal;
	return output;
}