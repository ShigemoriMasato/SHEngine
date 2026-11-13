struct PSInput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL0;
    float3 localPosition : TEXCOORD1;
    float4 color : COLOR0;
    float4 param : TEXCOORD2;
};

struct PSOutput
{
    float4 color : SV_TARGET0;
    // Bloom Mask。初期実装ではゲーム画面と同じ色を出力する(見た目が揃った段階で発光対象を見直す)
    float4 bloomMask : SV_TARGET1;
};

PSOutput main(PSInput input)
{
    PSOutput output;

    // 簡易な平行光源(陰影が無いと球の形が分からないため)
    float3 normal = normalize(input.normal);
    float3 lightDirection = normalize(float3(0.3f, -1.0f, 0.4f));
    float diffuse = saturate(dot(normal, -lightDirection)) * 0.6f + 0.4f;

    // 仮モデル用の模様: モデル空間の8分割チェッカー。単色の球では転がりが見えないため、param.x で強さを指定する
    float3 octant = sign(input.localPosition);
    float checker = (octant.x * octant.y * octant.z) > 0.0f ? 1.0f : 0.55f;
    float pattern = lerp(1.0f, checker, saturate(input.param.x));

    float4 color = float4(input.color.rgb * diffuse * pattern, input.color.a);
    output.color = color;
    output.bloomMask = color;
    return output;
}
