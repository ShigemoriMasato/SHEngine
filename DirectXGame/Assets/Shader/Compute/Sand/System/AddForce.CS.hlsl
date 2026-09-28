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

cbuffer ForceInfo : register(b4)
{
    // いくつでdensityが1になるか
    int forceMax;
}

StructuredBuffer<float3> particlePositions : register(t0);
StructuredBuffer<uint> isAlive : register(t1);
StructuredBuffer<int> density : register(t2);

RWStructuredBuffer<float3> force : register(u0);

static const float3 gridIndexOffset[6] =
{
    float3(-1, 0, 0),
    float3(1, 0, 0),
    float3(0, -1, 0),
    float3(0, 1, 0),
    float3(0, 0, -1),
    float3(0, 0, 1)
};

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
    int centerDensity = density[gridIndex];
    
    float3 totalForce = float3(0, 0, 0);
    for (int i = 0; i < 6; i++)
    {
        float3 offset = gridIndexOffset[i];
        int neighborX = gridX + (int)offset.x;
        int neighborY = gridY + (int)offset.y;
        int neighborZ = gridZ + (int)offset.z;
        
        // グリッドの範囲外であればスキップ
        if (neighborX < 0 || neighborX >= gridWidth || neighborY < 0 || neighborY >= gridHeight || neighborZ < 0 || neighborZ >= gridDepth)
            continue;
        
        int neighborIndex = neighborZ * gridWidth * gridHeight + neighborY * gridWidth + neighborX;
        int neighborDensity = density[neighborIndex];
        
        // 密度の強さだけ力を加える
        if (neighborDensity < centerDensity)
        {
            float diff = (float) (centerDensity - neighborDensity) / (float) forceMax;
            totalForce += offset * diff;
        }
    }

    force[index] = totalForce;
}
