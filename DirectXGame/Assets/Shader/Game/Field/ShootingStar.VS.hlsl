struct VSInput
{
    float3 position : POSITION0;
};

struct VSOutput
{
    float4 position : SV_POSITION;
};

cbuffer Camera : register(b0)
{
    float4x4 vpMatrix;
}

cbuffer Scale : register(b1)
{
    float scale;
}

StructuredBuffer<float2> positions : register(t0);

VSOutput main(VSInput input, uint instanceID : SV_InstanceID)
{
    VSOutput output;
    
    float4x4 world = float4x4(
    scale, 0.0f, 0.0f, 0.0f,
    0.0f, scale, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f,
    positions[instanceID].x, positions[instanceID].y, 0.0f, 1.0f
    );
    
    output.position = mul(float4(input.position, 1.0f), mul(world, vpMatrix));
    return output;
}