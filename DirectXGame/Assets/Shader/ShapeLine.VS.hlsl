struct TransformMatrix
{
    float4x4 world;
};
StructuredBuffer<TransformMatrix> trans : register(t0);

cbuffer Camera : register(b0)
{
    float4x4 vp;
};

struct VertexShaderInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

VSOutput main(VertexShaderInput input, uint instanceId : SV_InstanceID)
{
    VSOutput output;
    output.position = mul(input.position, mul(trans[instanceId].world, vp));
    output.texcoord = input.texcoord;
    output.normal = normalize(mul(input.normal, (float3x3) trans[instanceId].world));
    return output;
}
