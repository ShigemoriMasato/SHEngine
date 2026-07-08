RWStructuredBuffer<uint32_t> freeList : register(u0);
RWStructuredBuffer<uint32_t> freeListIndex : register(u1);
RWStructuredBuffer<uint32_t> indexList : register(u2);
RWStructuredBuffer<float32_t3> positions : register(u3);
RWStructuredBuffer<float16_t4> colors : register(u4);

StructuredBuffer<float32_t3> vertexList : register(t0);