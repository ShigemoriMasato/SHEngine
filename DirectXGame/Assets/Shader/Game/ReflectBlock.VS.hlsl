struct VSInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
    float3 normal : NORMAL0;
    float3 world : WORLD0;
    uint colorID : COLOR0;
};

struct ParticleData
{
    float4x4 world;
    float4x4 wvp;
    uint colorID;
};

float4 ConvertColor(uint color)
{
    float4 result = float4(
        ((color >> 24) & 0xFF) / 255.0,
        ((color >> 16) & 0xFF) / 255.0,
        ((color >> 8) & 0xFF) / 255.0,
        (color & 0xFF) / 255.0);
    
    return result;
}

StructuredBuffer<ParticleData> data : register(t0);

float absOne(float value)
{
    if (value > 0)
    {
        return 1;
    }
    else
    {
        return -1;
    }
}

VSOutput main(VSInput input, uint instance : SV_InstanceID)
{
    VSOutput output;
    output.position = mul(input.position, data[instance].wvp);
    output.texCoord = input.texcoord;
    output.normal = mul(input.normal, (float3x3) data[instance].world);
    output.world = mul(input.position, data[instance].world).xyz;
    output.colorID = data[instance].colorID;
    return output;
}
