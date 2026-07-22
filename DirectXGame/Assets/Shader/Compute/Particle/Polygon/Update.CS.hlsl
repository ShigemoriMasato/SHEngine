RWStructuredBuffer<uint> freeList : register(u0);
RWStructuredBuffer<uint> freeListIndex : register(u1);
RWStructuredBuffer<float> currentTime : register(u2);
RWStructuredBuffer<float3> position : register(u3);
RWStructuredBuffer<float16_t4> color : register(u4);

StructuredBuffer<int> indexList : register(t0);

struct IgnoreBall
{
    float32_t3 position;
    float32_t radius;
};

StructuredBuffer<IgnoreBall> ignoreBallList : register(t1);
StructuredBuffer<float3> basePositions : register(t2);

cbuffer MaxParticle : register(b0) {
    uint maxParticleNum;
}

cbuffer LifeTime : register(b1) {
    float lifeTime;
}

cbuffer deltaTime : register(b2) {
    float deltaTime;
}

cbuffer IgnoreBallNum : register(b3)
{
    uint ignoreBallNum;
}

[numthreads(128, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID) {
    uint index = DTid.x;
    if (index >= maxParticleNum) return;

    currentTime[index] += deltaTime;
    int globalIndex = indexList[index];
    float3 pos = position[globalIndex];

    //Positionが初期値のままなら何もしない
    static const float minValue = 1.175494351E-38;
    if(pos.x == minValue) return;
    
    float baseDist = distance(pos, basePositions[index]);
    float3 baseDir = float3(0,0,0);
    float innnerSpeed = 0.0f;
    if (baseDist > 0.01f)
    {
        baseDir = normalize(basePositions[index] - pos);
        innnerSpeed = baseDist;
    }
    
    float3 velocity = baseDir * innnerSpeed;
    for (uint i = 0; i < ignoreBallNum; i++)
    {
        float dist = length(ignoreBallList[i].position - pos);
        float intensity = dist / ignoreBallList[i].radius;
        float outerSpeed = 1.0f / (intensity * intensity);
        float3 dir = normalize(pos - ignoreBallList[i].position);
        velocity += dir * outerSpeed;
    }
    
    //Pool側の情報をいじる場合はglobalを使用する
    position[globalIndex] += velocity * deltaTime;
    color[globalIndex].a = float16_t(1.0 - (currentTime[index] / lifeTime));

    if (currentTime[index] >= lifeTime) {
        uint freeIndex;
        InterlockedAdd(freeListIndex[0], 1, freeIndex);
        freeList[freeIndex + 1] = index;
        position[globalIndex] = float3(minValue, minValue, minValue);
    }
}
