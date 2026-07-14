RWStructuredBuffer<uint> freeList : register(u0);
RWStructuredBuffer<uint> freeListIndex : register(u1);
RWStructuredBuffer<float> currentTime : register(u2);
RWStructuredBuffer<float32_t3> position : register(u3);
RWStructuredBuffer<float16_t4> color : register(u4);

StructuredBuffer<float16_t3> velocity : register(t0);
StructuredBuffer<int> indexList : register(t1);

cbuffer MaxParticle : register(b0) {
    uint maxParticleNum;
}

cbuffer LifeTime : register(b1) {
    float lifeTime;
}

cbuffer deltaTime : register(b2) {
    float deltaTime;
}

[numthreads(128, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID) {
    uint index = DTid.x;
    if (index >= maxParticleNum) return;

    currentTime[index] += deltaTime;

    int globalIndex = indexList[index];

    //Positionが初期値のままなら何もしない
    static const float minValue = 1.175494351E-38;
    if(position[globalIndex].x == minValue) return; 

    //Pool側の情報をいじる場合はglobalを使用する
    position[globalIndex] += float32_t3(velocity[index]) * deltaTime;
    color[globalIndex].a = float16_t(1.0 - (currentTime[index] / lifeTime));

    if (currentTime[index] >= lifeTime) {
        uint freeIndex;
        InterlockedAdd(freeListIndex[0], 1, freeIndex);
        freeList[freeIndex + 1] = index;
        position[globalIndex] = float32_t3(minValue, minValue, minValue);
    }
}
