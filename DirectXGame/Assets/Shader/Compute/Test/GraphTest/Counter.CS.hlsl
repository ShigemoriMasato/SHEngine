// ComputeGraphのテスト用: uintのバッファを数える。floatのバッファ(stride 4)とは要素サイズが同じなのでつながるが、
// 要素サイズの違うバッファ(Pair: stride 8)はつなげないことを確認する

RWStructuredBuffer<uint> counter : register(u0);

[numthreads(64, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint count;
    uint stride;
    counter.GetDimensions(count, stride);
    if (DTid.x >= count)
    {
        return;
    }
    counter[DTid.x] += 1;
}
