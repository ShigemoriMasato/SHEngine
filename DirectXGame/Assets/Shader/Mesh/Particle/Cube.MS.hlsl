// A mesh group emits 32 cubes: 16 threads, 2 cubes per thread.
// Output is 256 vertices and 384 triangles, within the D3D12 mesh shader limits.
static const uint THREAD_GROUP_SIZE = 8;
static const uint CUBES_PER_THREAD = 2;
static const uint MS_GROUPS_PER_AS_THREAD = 64;
static const uint VERTICES_PER_CUBE = 8;
static const uint TRIANGLES_PER_CUBE = 12;
static const uint CUBES_PER_GROUP = THREAD_GROUP_SIZE * CUBES_PER_THREAD;

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

// MeshShaderのキャパで考えると21インスタンスまで
// 計算のコストを考えて16インスタンス描画

[outputtopology("triangle")]
[numthreads(THREAD_GROUP_SIZE, 1, 1)]
void main(
    in payload Payload asID,
    uint3 globalID : SV_DispatchThreadID,
    uint3 localID : SV_GroupThreadID,
    out vertices VertexOutput vertices[128],
    out indices uint3 triangles[192])
{
    SetMeshOutputCounts(
        VERTICES_PER_CUBE * CUBES_PER_GROUP,
        TRIANGLES_PER_CUBE * CUBES_PER_GROUP);

    // Cube corners, shared by the 12 triangles below.
    static const float3 cubeCorners[8] =
    {
        float3(-0.5f, -0.5f, -0.5f),
        float3(-0.5f,  0.5f, -0.5f),
        float3( 0.5f,  0.5f, -0.5f),
        float3( 0.5f, -0.5f, -0.5f),
        float3(-0.5f, -0.5f,  0.5f),
        float3(-0.5f,  0.5f,  0.5f),
        float3( 0.5f,  0.5f,  0.5f),
        float3( 0.5f, -0.5f,  0.5f)
    };

    static const uint3 cubeTriangles[12] =
    {
        uint3(0, 2, 1), uint3(0, 3, 2), // -Z
        uint3(4, 5, 6), uint3(4, 6, 7), // +Z
        uint3(0, 1, 5), uint3(0, 5, 4), // -X
        uint3(3, 7, 6), uint3(3, 6, 2), // +X
        uint3(0, 4, 7), uint3(0, 7, 3), // -Y
        uint3(1, 2, 6), uint3(1, 6, 5)  // +Y
    };

    [unroll]
    for (uint cubeInThread = 0; cubeInThread < CUBES_PER_THREAD; ++cubeInThread)
    {
        const uint particleID = asID.threadID * (MS_GROUPS_PER_AS_THREAD * CUBES_PER_GROUP)
            + globalID.x * CUBES_PER_THREAD + cubeInThread;
        const uint cubeIndex = localID.x * CUBES_PER_THREAD + cubeInThread;
        const uint vertexOffset = cubeIndex * VERTICES_PER_CUBE;
        const uint triangleOffset = cubeIndex * TRIANGLES_PER_CUBE;
        const bool particleValid = particleID < (uint)maxNum;

        [unroll]
        for (uint vertex = 0; vertex < VERTICES_PER_CUBE; ++vertex)
        {
            vertices[vertexOffset + vertex].instanceID = particleID;
        }

        [unroll]
        for (uint tri = 0; tri < TRIANGLES_PER_CUBE; ++tri)
        {
            triangles[triangleOffset + tri] = uint3(
                vertexOffset + cubeTriangles[tri].x,
                vertexOffset + cubeTriangles[tri].y,
                vertexOffset + cubeTriangles[tri].z);
        }

        if (!particleValid || scales[particleID] < 1e-6f)
        {
            [unroll]
            for (uint vertex = 0; vertex < VERTICES_PER_CUBE; ++vertex)
            {
                vertices[vertexOffset + vertex].position = float4(3.0f, 3.0f, 0.0f, 1.0f);
            }
            continue;
        }

        const float3 position = positions[particleID];
        const float size = scales[particleID];
        const uint parentIndex = parentIndices[particleID];

        float4x4 parentMatrix = float4x4(
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1);
        if (parentIndex != 0xffffffff)
        {
            parentMatrix = parentMatrices[parentIndex];
        }

        const float4x4 scaleMatrix = float4x4(
            size, 0, 0, 0,
            0, size, 0, 0,
            0, 0, size, 0,
            0, 0, 0, 1);
        const float4x4 translateMatrix = float4x4(
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            position.x, position.y, position.z, 1);
        const float4x4 wvpMatrix = mul(
            mul(mul(mul(scaleMatrix, billboardMatrix), translateMatrix), parentMatrix),
            vpMatrix);

        [unroll]
        for (uint vertex = 0; vertex < VERTICES_PER_CUBE; ++vertex)
        {
            vertices[vertexOffset + vertex].position = mul(
                float4(cubeCorners[vertex], 1.0f), wvpMatrix);
        }
    }
}
