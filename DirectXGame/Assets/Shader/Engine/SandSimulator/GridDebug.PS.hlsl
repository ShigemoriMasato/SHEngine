struct PSInput
{
    float3 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
    float3 normal : NORMAL0;
    uint instanceID : INSTANCE0;
};

struct PSOutput
{
    float4 color : SV_Target0;
};

cbuffer MaterialData : register(b0)
{
    float3 upperColor;
    float alpha;
    float3 lowerColor;
    float outlineWidth;
    float4 outlineColor;
};

StructuredBuffer<float> densityBuffer : register(t0);

PSOutput main(PSInput input)
{
    PSOutput output;
    
    //Outline
    if (input.texCoord.x < outlineWidth || input.texCoord.x > 1.0f - outlineWidth || input.texCoord.y < outlineWidth || input.texCoord.y > 1.0f - outlineWidth)
    {
        output.color = outlineColor;
        return output;
    }
    
    //Discard if alpha is too low
    if (alpha < 0.01f)
    {
        discard;
    }
    
    //MainColor Adjust
    float density = densityBuffer[input.instanceID];
    float3 color = lerp(lowerColor, upperColor, density);
    output.color = float4(color, alpha);
    
    return output;
}
