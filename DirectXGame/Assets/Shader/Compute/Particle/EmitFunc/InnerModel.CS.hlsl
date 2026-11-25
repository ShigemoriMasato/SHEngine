// モデル内部に一様な乱数位置でパーティクルを配置する (Particle::InnerModel::EmitInnerModel から一度だけ実行される)
// 八分木の葉セルを Alias テーブルで体積比に従って選び、セル内に乱数で点を打つ。
// 混在セルでは セル中心 -> 点 の線分が三角形を横切った符号付きの回数でワインディング数を求め、
// 0 (外部) ならセルの選択からやり直す。

cbuffer ParticleCount : register(b0)
{
    uint particleCount;
};

cbuffer Config : register(b1)
{
    uint emitCount;
    uint cellCount;
    uint seed;
    uint maxAttempts;

    float4 emitColor;
    float emitScale;
};

// C++ の Particle::InnerModel::Cell と同じレイアウト (32byte)
struct Cell
{
    float3 center;
    float half;
    uint triOffset;
    uint triCount;
    uint flags;
    int centerWinding; // セル中心のワインディング数 (0 以外なら内部)
};

static const uint kCellFlagMixed = 1u << 0;

StructuredBuffer<uint> indexList : register(t0);
StructuredBuffer<Cell> cells : register(t1);
StructuredBuffer<float3> triangles : register(t2);  // 3頂点ずつ
StructuredBuffer<float> aliasProbability : register(t3);
StructuredBuffer<uint> aliasIndex : register(t4);

RWStructuredBuffer<float3> positions : register(u0);
RWStructuredBuffer<float3> rotations : register(u1);
RWStructuredBuffer<float> scales : register(u2);
RWStructuredBuffer<float16_t4> color : register(u3);

uint Hash(inout uint x)
{
    x ^= x >> 16;
    x *= 0x7feb352d;
    x ^= x >> 15;
    x *= 0x846ca68b;
    x ^= x >> 16;
    return x;
}

// [0, 1)
float Rand01(inout uint state)
{
    return (Hash(state) >> 8) * (1.0f / 16777216.0f);
}

// Möller–Trumbore。o + t*d (0 < t <= 1) と三角形の交差を符号付きで返す
// 交差しない: 0 / 表側から入る: +1 / 裏側から出る: -1 (C++ の IntersectTriangle と同じ規約)
int IntersectSegment(float3 o, float3 d, float3 v0, float3 v1, float3 v2)
{
    float3 e1 = v1 - v0;
    float3 e2 = v2 - v0;
    float3 p = cross(d, e2);
    float det = dot(e1, p);
    if (abs(det) < 1e-20f)
    {
        return 0;
    }
    float inv = 1.0f / det;
    float3 s = o - v0;
    float u = dot(s, p) * inv;
    if (u < 0.0f || u > 1.0f)
    {
        return 0;
    }
    float3 q = cross(s, e1);
    float v = dot(d, q) * inv;
    if (v < 0.0f || u + v > 1.0f)
    {
        return 0;
    }
    float t = dot(e2, q) * inv;
    if (t <= 0.0f || t > 1.0f)
    {
        return 0;
    }
    return det > 0.0f ? 1 : -1;
}

bool IsInsideMixedCell(Cell cell, float3 p)
{
    float3 d = p - cell.center;
    int winding = cell.centerWinding;
    for (uint i = 0; i < cell.triCount; ++i)
    {
        uint base = (cell.triOffset + i) * 3;
        winding += IntersectSegment(cell.center, d, triangles[base], triangles[base + 1], triangles[base + 2]);
    }
    return winding != 0;
}

[numthreads(128, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;
    if (index >= particleCount || index >= emitCount || cellCount == 0)
    {
        return;
    }

    uint state = seed ^ (index * 0x9E3779B9u);
    Hash(state);

    float3 result = cells[0].center;
    for (uint attempt = 0; attempt < maxAttempts; ++attempt)
    {
        // Alias 法: 一様に選んだ枠を、確率 aliasProbability で自分 / それ以外で相方にする
        uint cellIndex = min(uint(Rand01(state) * cellCount), cellCount - 1);
        if (Rand01(state) >= aliasProbability[cellIndex])
        {
            cellIndex = aliasIndex[cellIndex];
        }

        Cell cell = cells[cellIndex];
        float3 offset = float3(Rand01(state), Rand01(state), Rand01(state)) * 2.0f - 1.0f;
        float3 p = cell.center + offset * cell.half;

        // 上限まで外れ続けた場合の保険 (通常は起きない)
        result = cell.center;

        if ((cell.flags & kCellFlagMixed) == 0 || IsInsideMixedCell(cell, p))
        {
            result = p;
            break;
        }
    }

    uint localIndex = indexList[index];
    positions[localIndex] = result;
    rotations[localIndex] = float3(0.0f, 0.0f, 0.0f);
    scales[localIndex] = emitScale;
    color[localIndex] = float16_t4(emitColor.r, emitColor.g, emitColor.b, emitColor.a);
}
