
struct VSOutput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
    float3 normal : NORMAL0;
    float4 color : COLOR0;
    float4 outlineColor : OUT_COLOR0;
};

#define PSInput VSOutput
