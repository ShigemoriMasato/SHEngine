
struct PSInput
{
    float4 position : SV_POSITION;
    uint instanceID : INSTANCE0;
};

struct PSOutput
{
    float4 color : SV_TARGET;
};

StructuredBuffer<float16_t4> colors : register(t0);
StructuredBuffer<uint> types : register(t1);

PSOutput main(PSInput input)
{
    PSOutput output;
    output.color = colors[input.instanceID];
    if(output.color.a < 0.01f){
        output.color = float4(1, 0, 1, 1);
    }
    return output;
}
