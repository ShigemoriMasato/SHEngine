
cbuffer Camera : register(b0)
{
    float4x4 vp;
};

struct VertexShaderInput
{
    float3 position : POSITION0;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

VSOutput main(VertexShaderInput input, uint instanceID : SV_InstanceID)
{
    VSOutput output;
    output.position = mul(float4(input.position, 1.0f), vp);
    output.texcoord = float2(0.0f, 0.0f);
    output.normal = float3(0.0f, 0.0f, 1.0f);
    return output;
}
