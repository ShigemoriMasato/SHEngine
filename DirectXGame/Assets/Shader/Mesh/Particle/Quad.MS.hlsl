static const uint THREAD_GROUP_SIZE = 64;
static const uint AS_WAVE = 16;

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
    float3 cameraPosition;
};
cbuffer MaxNum : register(b2)
{
    int maxNum;
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
    const int vertexCount = 4;
    const int primitiveCount = 2;
    
    const uint threadID = globalID.x + THREAD_GROUP_SIZE * AS_WAVE * asID.threadID;
    const uint vertexOffset = localID.x * vertexCount;
    const uint triangleOffset = localID.x * primitiveCount;

    SetMeshOutputCounts(vertexCount * THREAD_GROUP_SIZE, primitiveCount * THREAD_GROUP_SIZE);
    
    if(threadID >= maxNum) {
        return;
    }

    vertices[vertexOffset + 0].instanceID = threadID;
    vertices[vertexOffset + 1].instanceID = threadID;
    vertices[vertexOffset + 2].instanceID = threadID;
    vertices[vertexOffset + 3].instanceID = threadID;
    
    triangles[triangleOffset + 0] = uint3(vertexOffset + 0, vertexOffset + 1, vertexOffset + 2);
    triangles[triangleOffset + 1] = uint3(vertexOffset + 1, vertexOffset + 3, vertexOffset + 2);

    static const float minValue = 1.175494351E-38;
    if(positions[threadID].x == minValue && positions[threadID].y == minValue && positions[threadID].z == minValue) {
        vertices[vertexOffset + 0].position = float4(0, 0, 0, 0);
        vertices[vertexOffset + 1].position = float4(0, 0, 0, 0);
        vertices[vertexOffset + 2].position = float4(0, 0, 0, 0);
        vertices[vertexOffset + 3].position = float4(0, 0, 0, 0);
        return;
    }
    
    float dist = length(positions[threadID] - cameraPosition);
    float adjustmentSize = size * (1.0f + dist * 0.03f);
    
    float4x4 scale = float4x4(adjustmentSize, 0, 0, 0,
                                0, adjustmentSize, 0, 0,
                                0, 0, adjustmentSize, 0,
                                0, 0, 0, 1);
    float4x4 translate = float4x4(1, 0, 0, 0,
                                0, 1, 0, 0,
                                0, 0, 1, 0,
                                positions[threadID].x, positions[threadID].y, positions[threadID].z, 1);
    
    float4x4 wvpMatrix = mul(mul(mul(scale, billboardMatrix), translate), vpMatrix);
    vertices[vertexOffset + 0].position = mul(float4(-0.5f, 0.5f, 0.0f, 1.0f), wvpMatrix);
    vertices[vertexOffset + 1].position = mul(float4(0.5f, 0.5f, 0.0f, 1.0f), wvpMatrix);
    vertices[vertexOffset + 2].position = mul(float4(-0.5f, -0.5f, 0.0f, 1.0f), wvpMatrix);
    vertices[vertexOffset + 3].position = mul(float4(0.5f, -0.5f, 0.0f, 1.0f), wvpMatrix);
}