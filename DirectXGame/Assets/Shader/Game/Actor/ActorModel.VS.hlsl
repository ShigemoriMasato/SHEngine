struct VSInput
{
    float3 position : POSITION0;
    float2 texCoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL0;
    float3 localPosition : TEXCOORD1;
    float4 color : COLOR0;
    float4 param : TEXCOORD2;
};

// C++側の ModelInstance と同じレイアウト
struct InstanceData
{
    float4x4 world;
    float4 color;
    float4 param;
};

StructuredBuffer<InstanceData> instances : register(t0);

cbuffer Camera : register(b0)
{
    float4x4 vp;
}

VSOutput main(VSInput input, uint instanceID : SV_InstanceID)
{
    VSOutput output;
    InstanceData instance = instances[instanceID];

    float4 worldPosition = mul(float4(input.position, 1.0f), instance.world);
    output.position = mul(worldPosition, vp);
    // 一様スケールのみを想定しているため、法線はワールド行列の回転部分で変換して正規化する
    output.normal = mul(input.normal, (float3x3) instance.world);
    output.localPosition = input.position;
    output.color = instance.color;
    output.param = instance.param;
    return output;
}
