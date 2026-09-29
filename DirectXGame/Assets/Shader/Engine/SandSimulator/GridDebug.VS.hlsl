struct VSInput
{
    float3 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

struct VSOutput
{
    float3 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
    float3 normal : NORMAL0;
    uint instanceID : INSTANCE0;
};

struct ParticleData
{
    float4x4 world;
    float4x4 wvp;
};

StructuredBuffer<ParticleData> data : register(t0);

cbuffer GridCount : register(b0)
{
    int gridCountX;
    int gridCountY;
    int gridCountZ;
}

cbuffer GridSize : register(b1)
{
    float gridSize;
}

cbuffer GridOffset : register(b2)
{
    float3 gridOffset;
}

VSOutput main(VSInput input, uint instance : SV_InstanceID)
{
    int x = instance % gridCountX;
    int y = (instance / gridCountX) % gridCountY;
    int z = instance / (gridCountX * gridCountY);
    
    float4x4 local = float4x4(
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    gridOffset.x + x * gridSize, gridOffset.y + y * gridSize, gridOffset.z + z * gridSize, 1.0f
    );
    
    VSOutput output;
    output.position = mul(mul(float4(input.position, 1.0f), local), data[instance].wvp);
    output.texCoord = input.texcoord;
    output.normal = mul(input.normal, (float3x3) data[instance].world);
    output.instanceID = instance;
    return output;
}
