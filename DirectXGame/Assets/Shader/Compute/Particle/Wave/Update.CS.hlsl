cbuffer ParticleNum : register(b0)
{
    uint maxNum;
};
cbuffer deltaTime : register(b1)
{
    float deltaTime;
};
cbuffer LifeTime : register(b2)
{
    float lifetime;
};
cbuffer UpdateData : register(b3)
{
    float4x4 parentMatrix;
    float colorIntensity;
};

struct Wave
{
    float3 position;
    float speed;
    
    float3 color;
    float intensity;
    
    float lifetime;
    float decayRate;
    float maxlifetime;
    float thickness;
};

StructuredBuffer<Wave> waves : register(t0);
static const uint kWaveNum = 16;

StructuredBuffer<int> indexList : register(t1);

RWStructuredBuffer<int> freeList : register(u0);
RWStructuredBuffer<int> freeListIndex : register(u1);
RWStructuredBuffer<float32_t3> positions : register(u2);
RWStructuredBuffer<float16_t4> colors : register(u3);
RWStructuredBuffer<float16_t3> velocities : register(u4);
RWStructuredBuffer<float16_t> currentTimes : register(u5);
RWStructuredBuffer<float16_t3> basePositions : register(u6);
RWStructuredBuffer<float16_t3> baseColors : register(u7);

static const float32_t kMinValue32 = 1.175494351E-38;
static const float16_t kMinValue16 = 6.103515625E-05;
static const float32_t pie = 3.1415926535897932384626433832795;

[numthreads(128, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;
    if (index >= maxNum)
    {
        return;
    }
    
    if (basePositions[index].x == kMinValue16)
    {
        return;
    }
    
    int globalIndex = indexList[index];
    basePositions[index] += velocities[index] * float16_t(deltaTime);
    
    //波の処理
    float32_t3 pos = basePositions[index];
    float16_t3 col = baseColors[index];
    for (int i = 0; i < kWaveNum; ++i)
    {
        if (waves[i].lifetime >= waves[i].maxlifetime)
        {
            continue;
        }
        
        float dist = length(pos.xz - waves[i].position.xz);
        float radius = waves[i].speed * waves[i].lifetime;
        float diff = dist - radius;
        float range = waves[i].thickness;
        
        if (diff > -range && diff < range)
        {
            float decay = 1.0f - (waves[i].lifetime / waves[i].maxlifetime);
            float intensity = sqrt((1.0f - abs(diff) / range) * waves[i].intensity * decay);
            float timeIntensity = 1.0f - (waves[i].lifetime / waves[i].maxlifetime);
            pos += float16_t3(0, intensity * sqrt(timeIntensity), 0);
            col += float16_t3(waves[i].color * intensity * colorIntensity * timeIntensity);
        }
    }
    
    //出力先に値を書き込む
    float16_t normalzed = currentTimes[index] / float16_t(lifetime);
    float16_t alpha = float16_t(abs((normalzed * 2) - 1));
    colors[globalIndex] = float16_t4(col, alpha);
    positions[globalIndex] = mul(float4(pos, 1.0f), parentMatrix).xyz;
    
    currentTimes[index] += float16_t(deltaTime);
    if ((currentTimes[index] >= float16_t(lifetime)))
    {
        uint freeIndex;
        InterlockedAdd(freeListIndex[0], 1, freeIndex);
        freeList[freeIndex + 1] = index;
        positions[globalIndex] = float32_t3(kMinValue32, kMinValue32, kMinValue32);
        basePositions[index] = float16_t3(kMinValue16, kMinValue16, kMinValue16);
    }
}