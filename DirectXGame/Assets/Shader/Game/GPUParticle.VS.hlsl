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

StructuredBuffer<float3> positions : register(t0);
StructuredBuffer<uint> type : register(t1);

cbuffer Size : register(b0)
{
    float size;
};
cbuffer Camera : register(b1)
{
    float4x4 vpMatrix;
    float4x4 billboardMatrix;
};

VSOutput main(VSInput input, uint id : SV_InstanceID)
{
    VSOutput output;
    
    if (type[id] == 0)
    {
        output.position = float4(0, 0, 0, 0);
        output.normal = float3(0, 0, 0);
        output.texCoord = float2(0, 0);
        output.instanceID = id;
        return output;
    }
    
    float4x4 scale = float4x4(size, 0, 0, 0,
                                0, size, 0, 0,
                                0, 0, size, 0,
                                0, 0, 0, 1);
    float4x4 translate = float4x4(1, 0, 0, 0,
                                0, 1, 0, 0,
                                0, 0, 1, 0,
                                positions[id].x, positions[id].y, positions[id].z, 1);
    float4x4 world = mul(mul(scale, billboardMatrix), translate);
    output.position = mul(input.position, mul(world, vpMatrix));
    output.texCoord = input.texCoord;
    output.normal = input.normal;
    output.instanceID = id;
	return output;
}