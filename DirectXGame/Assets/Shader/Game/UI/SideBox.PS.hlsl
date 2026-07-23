struct PSInput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
};

struct PSOutput
{
    float4 color : SV_Target0;
};

cbuffer Color : register(b0)
{
    float4 color;
};

Texture2D<float4> texture : register(t0);
SamplerState gSampler : register(s0);

PSOutput main(PSInput input)
{
    PSOutput output;
    float4 texColor = texture.Sample(gSampler, input.texCoord);
    output.color = texColor * color;
    
    if (output.color.a < 0.01f)
    {
        discard;
    }
    
    return output;
}
