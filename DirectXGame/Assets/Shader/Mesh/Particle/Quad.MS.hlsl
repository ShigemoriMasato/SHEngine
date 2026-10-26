static const uint THREAD_GROUP_SIZE = 64;
static const uint MS_GROUPS_PER_AS_THREAD = 16;

struct VertexOutput
{
    float4 position : SV_POSITION;
    uint instanceID : INSTANCE0;
};

StructuredBuffer<float3> positions : register(t0);
StructuredBuffer<float> scales : register(t1);
StructuredBuffer<uint> parentIndices : register(t2);
StructuredBuffer<float4x4> parentMatrices : register(t3);

cbuffer MaxNum : register(b0)
{
    int maxNum;
};
cbuffer Camera : register(b1)
{
    float4x4 vpMatrix;
    float4x4 billboardMatrix;
};

struct Payload
{
    uint threadID;
};

[outputtopology("triangle")]
[numthreads(THREAD_GROUP_SIZE, 1, 1)]
void main(
in payload Payload asID, //ASのDispatchThreadID
uint3 globalID : SV_DispatchThreadID,
uint3 localID : SV_GroupThreadID,
out vertices VertexOutput vertices[256],
out indices uint3 triangles[128]
)
{
    const uint vertexCount = 4;
    const uint primitiveCount = 2;
    const uint particlesPerGroup = THREAD_GROUP_SIZE;

    SetMeshOutputCounts(vertexCount * particlesPerGroup, primitiveCount * particlesPerGroup);
    
    const uint threadID = asID.threadID * (MS_GROUPS_PER_AS_THREAD * particlesPerGroup) + globalID.x;
    const uint vertexOffset = localID.x * vertexCount;
    const uint triangleOffset = localID.x * primitiveCount;
        
    triangles[triangleOffset + 0] = uint3(vertexOffset + 0, vertexOffset + 1, vertexOffset + 2);
    triangles[triangleOffset + 1] = uint3(vertexOffset + 1, vertexOffset + 3, vertexOffset + 2);
        
    vertices[vertexOffset + 0].instanceID = threadID;
    vertices[vertexOffset + 1].instanceID = threadID;
    vertices[vertexOffset + 2].instanceID = threadID;
    vertices[vertexOffset + 3].instanceID = threadID;
    
    if (threadID >= maxNum)
    {
        vertices[vertexOffset + 0].position = float4(3.0f, 3.0f, 0.0f, 1.0f);
        vertices[vertexOffset + 1].position = float4(3.0f, 3.0f, 0.0f, 1.0f);
        vertices[vertexOffset + 2].position = float4(3.0f, 3.0f, 0.0f, 1.0f);
        vertices[vertexOffset + 3].position = float4(3.0f, 3.0f, 0.0f, 1.0f);
        return;
    }
        
    float3 position = positions[threadID];
    float size = scales[threadID];

    if (size < 1e-6f)
    {
        vertices[vertexOffset + 0].position = float4(3.0f, 3.0f, 0.0f, 1.0f);
        vertices[vertexOffset + 1].position = float4(3.0f, 3.0f, 0.0f, 1.0f);
        vertices[vertexOffset + 2].position = float4(3.0f, 3.0f, 0.0f, 1.0f);
        vertices[vertexOffset + 3].position = float4(3.0f, 3.0f, 0.0f, 1.0f);
        return;
    }

    float4x4 scale = float4x4(size, 0, 0, 0,
                                    0, size, 0, 0,
                                    0, 0, size, 0,
                                    0, 0, 0, 1);
    float4x4 translate = float4x4(1, 0, 0, 0,
                                    0, 1, 0, 0,
                                    0, 0, 1, 0,
                                    position.x, position.y, position.z, 1);

    float4x4 parentMatrix = float4x4(1, 0, 0, 0,
                                    0, 1, 0, 0,
                                    0, 0, 1, 0,
                                    0, 0, 0, 1);

    if (parentIndices[threadID] != 0xffffffff)
    {
        parentMatrix = parentMatrices[parentIndices[threadID]];
    }

    float4x4 wvpMatrix = mul(mul(mul(mul(scale, billboardMatrix), translate), parentMatrix), vpMatrix);
    vertices[vertexOffset + 0].position = mul(float4(-0.5f, 0.5f, 0.0f, 1.0f), wvpMatrix);
    vertices[vertexOffset + 1].position = mul(float4(0.5f, 0.5f, 0.0f, 1.0f), wvpMatrix);
    vertices[vertexOffset + 2].position = mul(float4(-0.5f, -0.5f, 0.0f, 1.0f), wvpMatrix);
    vertices[vertexOffset + 3].position = mul(float4(0.5f, -0.5f, 0.0f, 1.0f), wvpMatrix);
}

