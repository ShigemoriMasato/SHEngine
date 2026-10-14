cbuffer Counter : register(b0)
{
    uint particleCount;
};

cbuffer Scale : register(b1)
{
    float particleScale;
};

cbuffer GridConfig : register(b2)
{
    float3 gridOrigin;
    float scale;
    
    int width;
    int height;
    int depth;
    int particlePerCell;
    
    int cellCount;
};

struct Cell
{
    int index[4096];
};

StructuredBuffer<Cell> cellIndices : register(t0);
StructuredBuffer<int> cellCounts : register(t1);

RWStructuredBuffer<float3> positions : register(u0);
RWStructuredBuffer<float3> velocities : register(u1);

float3 RemoveForce(float3 a, float3 b)
{
    float doted = dot(a, b);
    float lengthSq = dot(b, b);
    
    if (lengthSq == 0.0f)
    {
        return a;
    }
    
    return a - b * (doted / lengthSq);
}

static const int checkCellCount = 6;
static const int3 nearCellIndex[checkCellCount] =
{
    { 0, 0, 1 },
    { 0, 0, -1 },
    { 0, 1, 0 },
    { 0, -1, 0 },
    { 1, 0, 0 },
    { -1, 0, 0 }
};

int GetCellIndex(int3 cell)
{
    return cell.x + cell.y * width + cell.z * width * height;
}

[numthreads(128, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;
    if (index >= particleCount)
    {
        return;
    }
    
    int i, j;
    float collisionDistance = particleScale * 1.2f;
    float3 position = positions[index];
    float3 local = position - gridOrigin;
    
    int3 cell;
    cell.x = (int) (local.x / scale);
    cell.y = (int) (local.y / scale);
    cell.z = (int) (local.z / scale);
    
    int3 checkCell[checkCellCount + 1];
    checkCell[0] = cell;
    for (i = 0; i < checkCellCount; ++i)
    {
        checkCell[i + 1] = cell + checkCell[i];
    }
    
    for (i = 0; i < checkCellCount + 1; ++i)
    {
        if (checkCell[i].x < 0 || checkCell[i].x >= width || checkCell[i].y < 0 || checkCell[i].y >= height || checkCell[i].z < 0 || checkCell[i].z >= depth)
        {
            continue;
        }
    
        int cellIndex = GetCellIndex(checkCell[i]);
        int particleCountInCell = min(cellCounts[cellIndex], particlePerCell);
    
        for (j = 0; j < particleCountInCell; ++j)
        {
            int otherIndex = cellIndices[cellIndex].index[j];
        
            if (otherIndex == index)
            {
                continue;
            }
        
            float3 diff = positions[otherIndex] - position;
            float dist = length(diff);
        
            if (dist < collisionDistance)
            {
                float3 normalizedDiff = normalize(diff);
                position -= normalizedDiff * (collisionDistance - dist) * 1.2f;
                velocities[index] = RemoveForce(velocities[index], normalizedDiff);
            }
        }
    }
}
