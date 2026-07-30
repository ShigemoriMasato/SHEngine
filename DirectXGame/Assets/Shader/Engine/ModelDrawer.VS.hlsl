struct VSInput
{
    float3 position : POSITION0;
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

StructuredBuffer<float4x4> world : register(t0);
StructuredBuffer<float4x4> parents : register(t1);

cbuffer Camera : register(b0)
{
    float4x4 vp;
}

cbuffer NodeIndex : register(b1)
{
    uint nodeIndex;
}

VSOutput main(VSInput input,
uint instanceID : SV_InstanceID)
{
    VSOutput output;
    float4x4 worldMatrix = mul(parents[nodeIndex], world[instanceID]);
    
    output.position = mul(float4(input.position, 1.0f), mul(world[instanceID], vp));
    output.texCoord = input.texCoord;
    output.normal = input.normal;
    output.instanceID = instanceID;
	return output;
}