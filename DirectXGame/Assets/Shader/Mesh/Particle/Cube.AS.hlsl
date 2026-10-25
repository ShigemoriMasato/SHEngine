struct Payload
{
    uint threadID;
};

cbuffer ParticleNum : register(b0)
{
    int maxNum;
};

[numthreads(1, 1, 1)]
void main(
    uint3 dispatchThreadID : SV_DispatchThreadID)
{
    Payload p;
    p.threadID = dispatchThreadID.x;
    DispatchMesh(64, 1, 1, p);
}
