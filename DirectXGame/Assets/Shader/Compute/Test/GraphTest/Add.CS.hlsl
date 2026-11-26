// ComputeGraphのテスト用: output[i] += add (Fillの後に実行すると実行順の確認になる)

cbuffer AddConfig : register(b0)
{
    float add;
    float3 padding;
}

RWStructuredBuffer<float> output : register(u0);

[numthreads(64, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint count;
    uint stride;
    output.GetDimensions(count, stride);
    if (DTid.x >= count)
    {
        return;
    }
    output[DTid.x] += add;
}
