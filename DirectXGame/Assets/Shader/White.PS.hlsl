struct VSOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

struct PSOutput
{
    float4 color : SV_TARGET0;
};

PSOutput main(VSOutput input)
{
    PSOutput output;
    output.color = float4(1.0f, 1.0f, 1.0f, 1.0f);
    return output;
}
