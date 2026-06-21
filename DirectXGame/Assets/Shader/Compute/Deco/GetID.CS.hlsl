cbuffer MousePos : register(b0)
{
    float2 mousePos;
}

cbuffer TextureIndex : register(b1)
{
    int textureIndex;
}

Texture2D textures[] : register(t8);
SamplerState pointSampler : register(s0);

RWStructuredBuffer<uint> idBuffer : register(u0);

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint2 pixelPos = uint2(mousePos.x, mousePos.y);
    float4 color = textures[textureIndex].Load(int3(pixelPos, 0));
    uint id = ((uint) round(color.r * 255.0f) << 0) | ((uint) round(color.g * 255.0f) << 8) | ((uint) round(color.b * 255.0f) << 16);
    idBuffer[0] = id;
}
