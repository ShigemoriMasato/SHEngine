RWStructuredBuffer<float3> position : register(u0);
RWStructuredBuffer<float4x4> wvp : register(u1);
RWStructuredBuffer<uint> type : register(u2);

cbuffer Camera : register(b0)
{
    float4x4 vpMatrix;
    float4x4 billboardMatrix;
};
cbuffer ParticleNum : register(b1)
{
    uint maxNum;
};
cbuffer Size : register(b2)
{
    float size;
};

[numthreads(256, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;
    if (index >= maxNum)
    {
        return;
    }
    
    wvp[index] = vpMatrix;
    return;
    
    if (type[index] == 0)
    {
        wvp[index] = float4x4(0, 0, 0, 0,
                            0, 0, 0, 0,
                            0, 0, 0, 0,
                            0, 0, 0, 1);
        return;
    }
    
    float4x4 scale = float4x4(size, 0, 0, 0,
                            0, size, 0, 0,
                            0, 0, size, 0,
                            0, 0, 0, size);
    float4x4 translate = float4x4(1, 0, 0, 0,
                                0, 1, 0, 0,
                                0, 0, 1, 0,
                                position[index].x, position[index].y, position[index].z, 1);
    float4x4 world = mul(mul(scale, billboardMatrix), translate);
    
    wvp[index] = mul(world, vpMatrix);
}