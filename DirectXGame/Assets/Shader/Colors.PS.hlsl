
struct PSInput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
    float3 normal : NORMAL0;
    uint instanceID : INSTANCE0;
};

struct PSOutput
{
    float4 color : SV_TARGET;
};

struct ColorBuffer
{
    float4 color;
};
StructuredBuffer<ColorBuffer> colors : register(t0);

PSOutput main(PSInput input)
{
    PSOutput output;
    output.color = colors[input.instanceID].color;
    return output;
}
