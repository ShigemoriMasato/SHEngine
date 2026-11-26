// ComputeGraphのテスト用: output[i] = input[i] * scale + offset

cbuffer FillConfig : register(b0)
{
    float scale;
    float offset;
    uint count;
    float padding;
}

StructuredBuffer<float> input : register(t0);
RWStructuredBuffer<float> output : register(u0);

[numthreads(64, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;
    if (index >= count)
    {
        return;
    }
    output[index] = input[index] * scale + offset;
}
