cbuffer GridInfo : register(b0)
{
    int gridWidth;
    int gridHeight;
};

RWStructuredBuffer<float> density : register(u0);

[numthreads(128, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID) {
    uint index = DTid.x;
    if (index >= gridWidth * gridHeight) 
        return;
    
    density[index] = 0.0f;
}
