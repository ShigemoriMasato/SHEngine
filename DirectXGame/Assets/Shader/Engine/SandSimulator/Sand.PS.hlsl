
struct PSInput
{
    float4 position : SV_POSITION;
};

cbuffer Color : register(b0)
{
    float4 color;
};

struct PSOutput
{
    float4 color : SV_TARGET0;
};

PSOutput main(PSInput input)
{
    PSOutput output;
    output.color = color;
    return output;
}
