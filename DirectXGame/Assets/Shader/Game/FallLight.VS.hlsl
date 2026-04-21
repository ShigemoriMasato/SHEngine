struct VSInput
{
    float4 position : POSITION0;
    float2 texCoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

struct VSOutput
{
    float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD0;
	float3 normal : NORMAL0;
    float4 color : COLOR0;
};

struct VSData
{
    float4x4 vp;
    
    float3 dir;
    float timer;
    
    float3 color;
    float lifeSpan;
    
    float3 firstPosition;
    float Speed;
};

StructuredBuffer<VSData> data : register(t0);

VSOutput main(VSInput input, uint instanceID : SV_InstanceID)
{
    float3 position = input.position.xyz;
    position += data[instanceID].firstPosition;
    
    float power = pow(saturate(-data[instanceID].timer / data[instanceID].lifeSpan + 1.0f), 3);
    
    float speed = data[instanceID].Speed * power;
    
    float3 move = speed * data[instanceID].dir;
    position += move;
    float4 color = float4(data[instanceID].color, power);
    
    VSOutput output;
    output.position = mul(float4(position, 1.0f), data[instanceID].vp);
    output.texCoord = input.texCoord;
    output.normal = input.normal;
    output.color = color;
	return output;
}
