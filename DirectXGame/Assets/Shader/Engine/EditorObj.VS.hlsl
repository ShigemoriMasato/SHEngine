struct VSInput
{
    float3 position : POSITION0;
    float2 texCoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
    float3 normal : NORMAL0;
    float4 colorID : COLOR0;
};

cbuffer Camera : register(b0)
{
    float4x4 vpMat;
};

StructuredBuffer<float4x4> worlds : register(t0);

// RGBのみでuint24_tとして扱う。それ以上詰め込むとオーバーフローする。
StructuredBuffer<uint> idBuffer : register(t1);

float4 EncodeIDToColor(uint id)
{
    float r = float(id & 0xFF) / 255.0f;
    float g = float((id >> 8) & 0xFF) / 255.0f;
    float b = float((id >> 16) & 0xFF) / 255.0f;
    return float4(r, g, b, 1.0f);
}

VSOutput main(VSInput input, uint instanceID : SV_InstanceID)
{
    VSOutput output;
    output.position = mul(float4(input.position, 1.0f), mul(worlds[instanceID], vpMat));
    output.texCoord = input.texCoord;
    output.normal = mul(float4(input.normal, 1.0f), worlds[instanceID]).xyz;
    output.colorID = EncodeIDToColor(idBuffer[instanceID]);
    return output;
}