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

cbuffer Data : register(b0)
{
    float3 cameraPos;
    float strength;
}

StructuredBuffer<ColorMap> colorMap : register(t0);
TextureCube<float4> texture : register(t1);

SamplerState gSampler : register(s0);

PSOutput main(PSInput input)
{
    PSOutput output;
    
    float outlineWidth = 0.05f;
    
    output.color = colorMap[input.colorID].color;
    
    float2 texcoord = input.texCoord;
    while (texcoord.x > 1.0f)
    {
        texcoord.x -= 1.0f;
    }
    while(texcoord.x < 0.0f)
    {
        texcoord.x += 1.0f;
    }
    while (texcoord.y > 1.0f)
    {
        texcoord.y -= 1.0f;
    }
    while (texcoord.y < 0.0f)
    {
        texcoord.y += 1.0f;
    }
    
    if (input.texCoord.x < outlineWidth || input.texCoord.x > 1.0f - outlineWidth ||
       input.texCoord.y < outlineWidth || input.texCoord.y > 1.0f - outlineWidth)
    {
        output.color = colorMap[input.colorID].outlineColor;
    }
    
    float3 cameraToPosition = normalize(input.world - cameraPos);
    float3 reflectedVector = reflect(cameraToPosition, input.normal);
    float3 texColor = texture.Sample(gSampler, reflectedVector).rgb;
    
    output.color.rgb += texColor * strength;
    
    if(output.color.a < 0.02f)
    {
        discard;
    }
    
    return output;
}
