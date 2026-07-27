struct PSInput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
    uint instanceID : INSTANCE0;
    uint primitiveID : SV_PrimitiveID;
};

struct PSOutput
{
    float4 color : SV_TARGET;
};

struct MaterialData
{
    float4 baseColor;
    float metallic;
    float roughness;
    int textureIndex;
    int normalTexture;
};

StructuredBuffer<MaterialData> materials : register(t0);
StructuredBuffer<uint> materialIndices : register(t1);

Texture2D<float4> textures[] : register(t8);
SamplerState gSampler: register(s0);

PSOutput main(PSInput input)
{
    PSOutput output;
    MaterialData material = materials[materialIndices[input.primitiveID]];
    
    float4 baseColor = material.baseColor;
    float4 texColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
    if (material.textureIndex >= 0)
    {
        texColor = textures[material.textureIndex].Sample(gSampler, input.texcoord);
    }
    
    //その他マテリアル処理はいつかやる
    
    output.color = baseColor * texColor;
    
    return output;
}
