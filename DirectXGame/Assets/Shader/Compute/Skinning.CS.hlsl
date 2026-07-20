struct Vertex
{
    float3 position;
    float2 texcoord;
    float3 normal;
};

struct VertexInfluence
{
    float4 weight;
    int4 index;
};

struct SkinningInformation
{
    uint numVertices;
};

struct Well
{
    float4x4 skeletonSpaceMatrix;
    float4x4 skeletonSpaceInverseTransposeMatrix;
};

StructuredBuffer<Well> matrices : register(t0);
StructuredBuffer<Vertex> vertices : register(t1);
StructuredBuffer<VertexInfluence> influences : register(t2);
RWStructuredBuffer<Vertex> output : register(u0);
ConstantBuffer<SkinningInformation> skinninfInfo : register(b0);

[numthreads(1024, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint vertexIndex = DTid.x;
    if (vertexIndex >= skinninfInfo.numVertices)
    {
        return;
    }
    
    //必要なデータの抽出
    Vertex input = vertices[vertexIndex];
    VertexInfluence influence = influences[vertexIndex];
    
    //Skinning後の頂点
    Vertex skinned;
    skinned.texcoord = input.texcoord;
    
    if (influence.weight.x + influence.weight.y + influence.weight.z + influence.weight.w == 0.0)
    {
        skinned.position = input.position;
        skinned.normal = input.normal;
    }
    else
    {
        float4 skinnedPosition = float4(0.0, 0.0, 0.0, 0.0);
        float3 skinnedNormal = float3(0.0, 0.0, 0.0);
        skinnedPosition = mul(float4(input.position, 1.0f), matrices[influence.index.x].skeletonSpaceMatrix) * influence.weight.x;
        skinnedPosition += mul(float4(input.position, 1.0f), matrices[influence.index.y].skeletonSpaceMatrix) * influence.weight.y;
        skinnedPosition += mul(float4(input.position, 1.0f), matrices[influence.index.z].skeletonSpaceMatrix) * influence.weight.z;
        skinnedPosition += mul(float4(input.position, 1.0f), matrices[influence.index.w].skeletonSpaceMatrix) * influence.weight.w;

        skinnedNormal = normalize(mul(input.normal, (float3x3) matrices[influence.index.x].skeletonSpaceInverseTransposeMatrix)) * influence.weight.x;
        skinnedNormal += normalize(mul(input.normal, (float3x3) matrices[influence.index.y].skeletonSpaceInverseTransposeMatrix)) * influence.weight.y;
        skinnedNormal += normalize(mul(input.normal, (float3x3) matrices[influence.index.z].skeletonSpaceInverseTransposeMatrix)) * influence.weight.z;
        skinnedNormal += normalize(mul(input.normal, (float3x3) matrices[influence.index.w].skeletonSpaceInverseTransposeMatrix)) * influence.weight.w;
        
        skinned.position = skinnedPosition.xyz;
        skinned.normal = skinnedNormal;
    }
    
    output[vertexIndex] = skinned;
}