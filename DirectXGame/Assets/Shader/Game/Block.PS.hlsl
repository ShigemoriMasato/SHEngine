#include "Block.hlsli"

struct PSOutput
{
    float4 color : SV_Target0;
};

PSOutput main(PSInput input)
{
    PSOutput output;
    
    float outlineWidth = 0.05f;
    
    output.color = input.color;
    
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
        output.color = input.outlineColor;
    }
    
    if(output.color.a < 0.02f)
    {
        discard;
    }
    
    return output;
}
