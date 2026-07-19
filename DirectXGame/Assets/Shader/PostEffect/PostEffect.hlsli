
struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

#define PixelShaderInput VertexShaderOutput

struct PixelShaderOutput
{
    float4 color : SV_Target;
};
