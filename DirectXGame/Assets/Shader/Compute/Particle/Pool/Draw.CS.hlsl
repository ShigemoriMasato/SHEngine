RWTexture2D<uint> redTexture : register(u0);
RWTexture2D<uint> greenTexture : register(u1);
RWTexture2D<uint> blueTexture : register(u2);
Texture2D<float4> depthTexture : register(t0);

StructuredBuffer<float32_t3> positions : register(t1);
StructuredBuffer<float16_t4> colors : register(t2);

cbuffer ExecuteOffset : register(b0)
{
    uint offset;
    uint maxCount;
}

cbuffer Camera : register(b1)
{
    float4x4 vpMatrix;
}

#define THREADGROUP_SIZE 128

[numthreads(THREADGROUP_SIZE, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint id = DTid.x + offset * THREADGROUP_SIZE;
    static const float32_t minValue = 1.175494351E-38;
    //死んでいる場合はxyzがminValueになっているので描画しない(とりあえずxだけで判定)
    if (id >= maxCount || positions[id].x == minValue)
        return;
    
    static const float width = 1280.0f;
    static const float height = 720.0f;
    
    float4 screenPos = mul(float4(positions[id], 1.0f), vpMatrix);
    float3 ndcPos = screenPos.xyz / screenPos.w;
    uint2 pixelCoord = uint2((ndcPos.xy * 0.5f + 0.5f) * float2(width, height));


    
    //画面外カリング & 深度テスト
    float depth = depthTexture.Load(int3(pixelCoord, 0)).r;
    
    if (ndcPos.z < 0.0f || ndcPos.z > 1.0f || ndcPos.z > depth)
        return;
    
    float3 color = colors[id].rgb * colors[id].a;

    //オーバーフローを防ぐため、uint16_t4の形で記述する。
    uint3 color16;
    color16.r = (uint32_t)(color.r * 65535.0f);
    color16.g = (uint32_t)(color.g * 65535.0f);
    color16.b = (uint32_t)(color.b * 65535.0f);

    InterlockedAdd(redTexture[int2(pixelCoord)], color16.r);
    InterlockedAdd(greenTexture[int2(pixelCoord)], color16.g);
    InterlockedAdd(blueTexture[int2(pixelCoord)], color16.b);
}