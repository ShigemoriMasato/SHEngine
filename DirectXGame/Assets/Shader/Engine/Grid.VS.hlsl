struct VSInput
{
    float3 position : POSITION0;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float4 color : COLOR0;
};

cbuffer MatrixBuffer : register(b0)
{
    float4x4 vp;
}

struct LineBuffer
{
    float3 start;
    float padding;
    float3 end;
    uint color;
};

float4 ConvertColor(uint color)
{
    float4 result;
    result.r = ((color >> 24) & 0xFF) / 255.0f;
    result.g = ((color >> 16) & 0xFF) / 255.0f;
    result.b = ((color >> 8) & 0xFF) / 255.0f;
    result.a = ((color >> 0) & 0xFF) / 255.0f;
    return result;
}

StructuredBuffer<LineBuffer> lineBuffer : register(t0);

VSOutput main(VSInput input, uint id : SV_InstanceID, uint vertex : SV_VertexID)
{
    VSOutput output;
    
    if (vertex == 0)
    {
        output.position = mul(float4(lineBuffer[id].start, 1.0f), vp);
        output.color = ConvertColor(lineBuffer[id].color);
        return output;
    }
    
    output.position = mul(float4(lineBuffer[id].end, 1.0f), vp);
    output.color = ConvertColor(lineBuffer[id].color);
    
	return output;
}