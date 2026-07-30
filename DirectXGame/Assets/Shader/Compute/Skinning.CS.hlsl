struct Vertex
{
    float3 position;
    float2 texcoord;
    float3 normal;
};

struct VertexInfluence
{
    int4 index;
    float4 weight;
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

StructuredBuffer<VertexInfluence> influences : register(t0);
StructuredBuffer<Well> matrices : register(t1);
StructuredBuffer<float3> positions : register(t2);
StructuredBuffer<float3> normals : register(t3);
RWStructuredBuffer<float3> outputPos : register(u0);
RWStructuredBuffer<float3> outputNorm : register(u1);
ConstantBuffer<SkinningInformation> skinningInfo : register(b0);

[numthreads(128, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint vertexIndex = DTid.x;
    if (vertexIndex >= skinningInfo.numVertices)
    {
        return;
    }
    
    //必要なデータの抽出
    float3 originPos = positions[vertexIndex];
    float3 originNorm = normals[vertexIndex];
    VertexInfluence influence = influences[vertexIndex];
    
    //Skinning後の頂点
    Vertex skinned;
    
    if (influence.weight.x + influence.weight.y + influence.weight.z + influence.weight.w == 0.0)
    {
        skinned.position = originPos;
        skinned.normal = originNorm;
    }
    else
    {
        float4 skinnedPosition = float4(0.0, 0.0, 0.0, 0.0);
        float3 skinnedNormal = float3(0.0, 0.0, 0.0);
        skinnedPosition = mul(float4(originPos, 1.0f), matrices[influence.index.x].skeletonSpaceMatrix) * influence.weight.x;
        skinnedPosition += mul(float4(originPos, 1.0f), matrices[influence.index.y].skeletonSpaceMatrix) * influence.weight.y;
        skinnedPosition += mul(float4(originPos, 1.0f), matrices[influence.index.z].skeletonSpaceMatrix) * influence.weight.z;
        skinnedPosition += mul(float4(originPos, 1.0f), matrices[influence.index.w].skeletonSpaceMatrix) * influence.weight.w;

        skinnedNormal = normalize(mul(originNorm, (float3x3) matrices[influence.index.x].skeletonSpaceInverseTransposeMatrix)) * influence.weight.x;
        skinnedNormal += normalize(mul(originNorm, (float3x3) matrices[influence.index.y].skeletonSpaceInverseTransposeMatrix)) * influence.weight.y;
        skinnedNormal += normalize(mul(originNorm, (float3x3) matrices[influence.index.z].skeletonSpaceInverseTransposeMatrix)) * influence.weight.z;
        skinnedNormal += normalize(mul(originNorm, (float3x3) matrices[influence.index.w].skeletonSpaceInverseTransposeMatrix)) * influence.weight.w;
        
        skinned.position = skinnedPosition.xyz;
        skinned.normal = skinnedNormal;
    }
    
    outputPos[vertexIndex] = skinned.position;
    outputNorm[vertexIndex] = skinned.normal;
}