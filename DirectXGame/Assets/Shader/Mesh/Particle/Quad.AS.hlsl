struct Payload
{
    uint threadID;
};

[numthreads(1, 1, 1)]
void main(
    uint3 dispatchThreadID : SV_DispatchThreadID)
{
    Payload p;
    p.threadID = dispatchThreadID.x;
    // Mesh Shaderを1グループ起動
    DispatchMesh(16, 1, 1, p);
}
