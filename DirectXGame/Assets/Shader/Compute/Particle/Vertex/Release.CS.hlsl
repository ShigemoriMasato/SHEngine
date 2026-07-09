RWStructuredBuffer<uint32_t> freeList : register(u0);
RWStructuredBuffer<uint32_t> freeListIndex : register(u1);
RWStructuredBuffer<uint32_t> indexList : register(u2);
RWStructuredBuffer<float32_t3> positions : register(u3);
RWStructuredBuffer<float16_t4> colors : register(u4);

cbuffer VertexNum : register(b0) {
    uint vertexNum;
}

//このシェーダーを実行中は、パーティクルの増加処理を行わない方がいい気がする。検証してないけど

[numthreads(128, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID) {
    uint index = DTid.x;
    if (index >= vertexNum) return;

    //値を初期値に戻す
    static const float minValue = 1.175494351E-38;
    uint id = indexList[index];
    positions[id] = float32_t3(minValue, minValue, minValue);
    colors[id] = float16_t4(0, 0, 0, 0);
    
    //FreeListに戻す
    uint freeIndex;
    InterlockedAdd(freeListIndex[0], 1, freeIndex);

    freeList[freeIndex] = id;
}
