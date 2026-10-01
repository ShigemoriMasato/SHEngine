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

cbuffer MatrixData : register(b0)
{
    float4x4 vp;
};

cbuffer GridCount : register(b1)
{
    int gridCountX;
    int gridCountY;
    int gridCountZ;
}

cbuffer GridSize : register(b2)
{
    float gridSize;
}

VSOutput main(VSInput input, uint instance : SV_InstanceID)
{
    int x = instance % gridCountX;
    int y = (instance / gridCountX) % gridCountY;
    int z = instance / (gridCountX * gridCountY);
    
    float4x4 world = float4x4(
    gridSize, 0.0f, 0.0f, 0.0f,
    0.0f, gridSize, 0.0f, 0.0f,
    0.0f, 0.0f, gridSize, 0.0f,
    x * gridSize, y * gridSize, z * gridSize, 1.0f
    );
    
    // this position's anchor is at the center of the cube. so it needs to be adjust to the corner(left, bottom, back) of the cube.
    float4 adjustmentPosition = float4(input.position + float3(0.5f, 0.5f, 0.5f), 1.0f);
    
    VSOutput output;
    output.position = mul(mul(float4(input.position, 1.0f), world), vp);
    output.texCoord = input.texcoord;
    output.normal = mul(input.normal, (float3x3) world);
    output.instanceID = instance;
    return output;
}
