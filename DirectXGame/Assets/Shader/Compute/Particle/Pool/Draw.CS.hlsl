RWTexture2D<uint16_t> outputTexture : register(u0);
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
    if (id >= maxCount)
        return;
    
    float4 screenPos = mul(positions[id], vpMatrix);
    float3 ndcPos = screenPos.xyz / screenPos.w;
    uint2 pixelCoord = uint2((ndcPos.xy * 0.5f + 0.5f) * float2(width, height));
    
    static const float width = 1280.0f;
    static const float height = 720.0f;
    
    //画面外カリング & 深度テスト
    float depth = depthTexture.Load(int3(pixelCoord, 0)).r;
    
    if (ndcPos.x < -1.0f || ndcPos.x > 1.0f || ndcPos.y < -1.0f || ndcPos.y > 1.0f || ndcPos.z < 0.0f || ndcPos.z > 1.0f || ndcPos.z > depth)
        return;
    
    float3 color = colors[id].rgb + colors[id].a;
    
    outputTexture[pixelCoord] += float4(color, 0.0f);
}