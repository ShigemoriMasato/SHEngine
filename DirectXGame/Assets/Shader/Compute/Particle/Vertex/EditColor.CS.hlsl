RWStructuredBuffer<float16_t4> colors : register(u0);

StructuredBuffer<uint32_t> indexList : register(t0);

cbuffer VertexNum : register(b0) {
    uint vertexNum;
}

cbuffer Color : register(b1) {
    float32_t4 color;
}

[numthreads(128, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID) {
    uint index = DTid.x;
    if (index >= vertexNum) return;

    uint id = indexList[index];
    colors[id] = float16_t4(color);
}
