
struct VSOutput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
    float3 normal : NORMAL0;
    float3 world : WORLD0;
    uint colorID : COLOR0;
};

#define PSInput VSOutput
