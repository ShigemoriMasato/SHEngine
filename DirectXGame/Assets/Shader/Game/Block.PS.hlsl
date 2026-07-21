#include "Block.hlsli"

struct PSOutput
{
    float4 color : SV_Target0;
};

struct ColorMap
{
    float4 color;
    float4 outlineColor;
};

StructuredBuffer<ColorMap> colorMap : register(t0);

PSOutput main(PSInput input)
{
    PSOutput output;
    
    float outlineWidth = 0.05f;
    
    output.color = colorMap[input.colorID].color;
    
    float2 texcoord = input.texCoord;
    texcoord = fmod(texcoord, 1.0f);
    
    if (input.texCoord.x < outlineWidth || input.texCoord.x > 1.0f - outlineWidth ||
       input.texCoord.y < outlineWidth || input.texCoord.y > 1.0f - outlineWidth)
    {
        output.color = colorMap[input.colorID].outlineColor;
    }
    
    if (output.color.a < 0.01f)
    {
        discard;
    }
    
    return output;
}
