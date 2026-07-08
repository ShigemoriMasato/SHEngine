struct VertexOutput
{
    float4 position : SV_POSITION;
    uint instanceID : INSTANCE0;
};

StructuredBuffer<float3> positions : register(t0);

cbuffer Size : register(b0)
{
    float size;
};
cbuffer Camera : register(b1)
{
    float4x4 vpMatrix;
    float4x4 billboardMatrix;
};
cbuffer MaxNum : register(b2)
{
    int maxNum;
};
cbuffer ExecuteOffset : register(b3)
{
    int executeOffset;
};

[outputtopology("triangle")]
[numthreads(128, 1, 1)]
void main(
uint3 id : SV_DispatchThreadID,
out vertices VertexOutput vertices[4], 
out indices uint3 triangles[2]
)
{
    int vertexCount = 4;
    int primitiveCount = 2;
    
    uint threadID = id.x + executeOffset;

    float minValue = 1.175494351E-38;
    
    if (threadID >= maxNum || positions[threadID].x == minValue || positions[threadID].y == minValue || positions[threadID].z == minValue)
    {
        vertexCount = 0;
        primitiveCount = 0;
    }
    
    SetMeshOutputCounts(vertexCount, primitiveCount);
    
    if (vertexCount == 0)
    {
        return;
    }
    
    float4x4 scale = float4x4(size, 0, 0, 0,
                                0, size, 0, 0,
                                0, 0, size, 0,
                                0, 0, 0, 1);
    float4x4 translate = float4x4(1, 0, 0, 0,
                                0, 1, 0, 0,
                                0, 0, 1, 0,
                                positions[threadID].x, positions[threadID].y, positions[threadID].z, 1);
    float4x4 world = mul(mul(scale, billboardMatrix), translate);
    float4x4 wvpMatrix = mul(world, vpMatrix);
    vertices[0].position = mul(float4(-0.5f, 0.5f, 0.0f, 1.0f), wvpMatrix);
    vertices[1].position = mul(float4(0.5f, 0.5f, 0.0f, 1.0f), wvpMatrix);
    vertices[2].position = mul(float4(-0.5f, -0.5f, 0.0f, 1.0f), wvpMatrix);
    vertices[3].position = mul(float4(0.5f, -0.5f, 0.0f, 1.0f), wvpMatrix);
    
    vertices[0].instanceID = threadID;
    vertices[1].instanceID = threadID;
    vertices[2].instanceID = threadID;
    vertices[3].instanceID = threadID;
    
    triangles[0] = uint3(0, 1, 2);
    triangles[1] = uint3(1, 3, 2);
}