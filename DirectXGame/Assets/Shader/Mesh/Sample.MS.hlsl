struct VertexOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void main(
    out vertices VertexOutput vertices[3],
    out indices uint3 triangles[1]
)
{
    SetMeshOutputCounts(3, 1);

    vertices[0].position = float4(-0.5f, -0.5f, 0.0f, 1.0f);
    vertices[1].position = float4(0.0f, 0.5f, 0.0f, 1.0f);
    vertices[2].position = float4(0.5f, -0.5f, 0.0f, 1.0f);
    
    vertices[0].texcoord = float2(0.0f, 0.0f);
    vertices[1].texcoord = float2(0.5f, 1.0f);
    vertices[1].texcoord = float2(1.0f, 0.0f);

    vertices[0].normal = float3(0, 0, -1);
    vertices[1].normal = float3(0, 0, -1);
    vertices[2].normal = float3(0, 0, -1);

    triangles[0] = uint3(0, 1, 2);
}