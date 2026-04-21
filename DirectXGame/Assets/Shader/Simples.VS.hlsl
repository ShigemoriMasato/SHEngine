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
    uint instanceID : INSTANCE0;
};

struct MatrixBuffer
{
    float4x4 wvp;
};

StructuredBuffer<MatrixBuffer> matrices : register(t0);

VSOutput main(VSInput input, uint id : SV_InstanceID)
{
    VSOutput output;
    float4x4 wvp = matrices[id].wvp;
    output.position = mul(input.position, wvp);
    output.texCoord = input.texCoord;
    output.normal = input.normal;
    output.instanceID = id;
	return output;
}