struct PSInput
{
    float4 position : SV_POSITION;
};

struct PSOutput
{
    float4 color : SV_Target0;
    float4 height : SV_Target1;
};

cbuffer Color : register(b0)
{
    float4 color;
};

PSOutput main(PSInput input)
{
    PSOutput output;
    output.color = color;
    output.height = float4(0.1f, 0.0f, 0.0f, 1.0f);
    return output;
}
