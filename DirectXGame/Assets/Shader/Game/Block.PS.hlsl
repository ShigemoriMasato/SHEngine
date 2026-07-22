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

struct Material
{
    float3 color;
    float intensity;
    uint colorID;
};

StructuredBuffer<ColorMap> colorMap : register(t0);
StructuredBuffer<Material> material : register(t1);

PSOutput main(PSInput input)
{
    PSOutput output;
    
    float outlineWidth = 0.05f;
    
    Material mat = material[input.instanceID];
    
    output.color = colorMap[mat.colorID].color;
    
    
    float2 texcoord = input.texCoord;
    texcoord = fmod(texcoord, 1.0f);
    
    if (input.texCoord.x < outlineWidth || input.texCoord.x > 1.0f - outlineWidth ||
       input.texCoord.y < outlineWidth || input.texCoord.y > 1.0f - outlineWidth)
    {
        output.color = colorMap[mat.colorID].outlineColor;
    }
    
    float2 centere = float2(0.5f, 0.5f);
    float dist = distance(input.texCoord, centere);
    float adjustedIntensity = material[input.instanceID].intensity * (1.0f - (dist / 2.0f));
    output.color.rgb += material[input.instanceID].color * adjustedIntensity;
    
    if (output.color.a < 0.01f)
    {
        discard;
    }
    
    return output;
}
