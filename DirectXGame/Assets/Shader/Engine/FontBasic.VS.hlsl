
struct VSOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD1;
};

cbuffer MatrixBuffer : register(b0)
{
    float4x4 world;
    float4x4 vp;
}

struct VSInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

struct StringUV
{
    float2 uv0;
    float2 uv1;
    float advanceX;
    float bearingX;
    float bearingY;
    float descender;
};
StructuredBuffer<StringUV> gUVs : register(t0);

VSOutput main(VSInput input, uint instanceId : SV_InstanceID, uint vertexID : SV_VertexID)
{
    VSOutput output;
    const int gAtlasSize = 4096; // アトラスサイズ

    // 今描いている文字の Glyph
    StringUV g = gUVs[instanceId];

    // ===== サイズ復元 =====
    float2 glyphSizePx = (g.uv1 - g.uv0) * gAtlasSize / 2.0f;

    // ===== Xオフセット計算（簡易）=====
    float penX = 0.0;
    [loop]
    for (uint i = 0; i < instanceId; ++i)
    {
        penX += gUVs[i].advanceX;
    }

    // ===== Plane をスケール =====
    float2 pos;
    pos.x = (input.position.x + 0.5f) * glyphSizePx.x;
    pos.y = (input.position.y + 0.5f) * glyphSizePx.y + g.descender;

    // ===== 位置補正 =====
    float baseX = round(penX);
    pos.x += baseX + g.bearingX;

    // ===== 座標変換 =====
    output.position = mul(float4(pos, 0.0, 1.0), mul(world, vp));

    // ===== UV変換 =====
    output.texcoord = float2((vertexID == 0 || vertexID == 1 ? g.uv0.x : g.uv1.x), (vertexID == 0 || vertexID == 3 ? g.uv0.y : g.uv1.y));

    return output;
}