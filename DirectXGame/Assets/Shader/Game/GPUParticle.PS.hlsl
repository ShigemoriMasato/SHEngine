
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

StructuredBuffer<float4> colors : register(t0);
StructuredBuffer<uint> types : register(t1);

PSOutput main(PSInput input)
{
    PSOutput output;
    output.color = colors[input.instanceID];
    return output;
}
