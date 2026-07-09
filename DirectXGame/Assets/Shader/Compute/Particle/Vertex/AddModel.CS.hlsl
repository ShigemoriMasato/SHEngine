RWStructuredBuffer<int32_t> freeList : register(u0);
RWStructuredBuffer<int32_t> freeListIndex : register(u1);
RWStructuredBuffer<uint32_t> indexList : register(u2);
RWStructuredBuffer<float32_t3> positions : register(u3);
RWStructuredBuffer<float16_t4> colors : register(u4);

StructuredBuffer<float32_t3> vertexList : register(t0);

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

    int freeIndex;
    InterlockedAdd(freeListIndex[0], -1, freeIndex);

    if(freeIndex < 0){
        InterlockedAdd(freeListIndex[0], 1, freeIndex);
        return;
    }

    uint id = freeList[freeIndex];
    
    indexList[index] = id;
    positions[id] = vertexList[index];
    colors[id] = float16_t4(color);
}
