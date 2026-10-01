cbuffer ParticleCount : register(b0)
{
    int particleCount;
};

RWStructuredBuffer<int> freeList : register(u0);
RWStructuredBuffer<int> freeListIndex : register(u1);
RWStructuredBuffer<int> isAlive : register(u2);

[numthreads(128, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID) {
    uint index = DTid.x;
    
    // パーティクルの最大数を超えていたら終了
    if (index >= particleCount)
        return;
    
    if (index == 0)
    {
        freeListIndex[index] = particleCount - 1;
    }
    
    freeList[index] = index;
    isAlive[index] = 0;
}
