cbuffer ParticleCount : register(b0)
{
    int particleCount;
}

cbuffer GridCount : register(b1)
{
    int gridWidth;
    int gridHeight;
    int gridDepth;
}

cbuffer GridSize : register(b2)
{
    float gridCellSize;
}

cbuffer GridOffset : register(b3)
{
    float3 gridOffset;
}

StructuredBuffer<float3> particlePositions : register(t0);
StructuredBuffer<uint> isAlive : register(t1);

RWStructuredBuffer<int> density : register(u0);

[numthreads(128, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID) {
    uint index = DTid.x;
    
    // パーティクルの最大数を超えていたら終了
    if (index >= particleCount)
        return;
    
    // パーティクルが生きていなければ終了
    if (isAlive[index] == 0)
        return;
    
    float3 localPosition = particlePositions[index] - gridOffset;
    
    int gridX = (int) (localPosition.x / gridCellSize);
    int gridY = (int) (localPosition.y / gridCellSize);
    int gridZ = (int) (localPosition.z / gridCellSize);
    
    // グリッドの範囲外であれば終了
    if (gridX < 0 || gridX >= gridWidth || gridY < 0 || gridY >= gridHeight || gridZ < 0 || gridZ >= gridDepth)
        return;
    
    int gridIndex = gridZ * gridWidth * gridHeight + gridY * gridWidth + gridX;
    InterlockedAdd(density[gridIndex], 1);
}
