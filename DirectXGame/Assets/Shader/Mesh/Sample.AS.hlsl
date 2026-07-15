

[numthreads(1, 1, 1)]
void main(
    uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Mesh Shaderを1グループ起動
    DispatchMesh(1, 1, 1, 0);
}
