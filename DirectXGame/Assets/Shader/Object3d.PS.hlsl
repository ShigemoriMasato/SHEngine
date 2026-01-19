
struct VertexShaderOutput
{
    float32_t4 position : SV_POSITION;
    float32_t2 texcoord : TEXCOORD1;
    float32_t3 normal : NORMAL1;
    float32_t3 worldPosition : POSITION1;
};

struct Material
{
    float32_t4 color;
};
ConstantBuffer<Material> gMaterial : register(b0);

struct PSOutput
{
    float4 color : SV_TARGET0;
};

PSOutput main(VertexShaderOutput input)
{
    PSOutput output;
    output.color = gMaterial.color;
    return output;
}
