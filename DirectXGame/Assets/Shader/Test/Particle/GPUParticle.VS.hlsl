struct VSInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float4 color : COLOR0;
};

struct Particle
{
    float3 position;
    float lifetime;
    float3 scale;
    float currentTime;
    float4 color;
    float3 velocity;
};

StructuredBuffer<Particle> data : register(t0);

cbuffer Camera : register(b0)
{
    float4x4 vpMatrix;
    float4x4 billboardMatrix;
};

VSOutput main(VSInput input, uint id : SV_InstanceID)
{
    VSOutput output;
    
    float4x4 worldMatrix = billboardMatrix;
    worldMatrix[0].x *= data[id].scale.x;
    worldMatrix[1].y *= data[id].scale.y;
    worldMatrix[2].z *= data[id].scale.z;
    worldMatrix[3].xyz = data[id].position;
    
    output.position = mul(float4(input.position.xyz, 1.0f), mul(worldMatrix, vpMatrix));
    output.color = data[id].color;

	return output;
}