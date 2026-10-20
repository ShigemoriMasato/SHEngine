cbuffer Counter : register(b0)
{
    uint particleCount;
};

cbuffer Scale : register(b1)
{
    //直径
    float particleScale;
};

cbuffer GridConfig : register(b2)
{
    float3 gridOrigin;
    float scale;
    
    int width;
    int height;
    int depth;
    int cellCount;
};

cbuffer ColliderConfig : register(b3)
{
    float elasticity;
    float friction;
};

StructuredBuffer<uint> particleIndexForCell : register(t0);
StructuredBuffer<uint> beginIndexForCell : register(t1);
StructuredBuffer<uint> particleCountInCell : register(t2);
StructuredBuffer<float3> positions : register(t3);

RWStructuredBuffer<float3> outputPositions : register(u0);
RWStructuredBuffer<float3> velocities : register(u1);

float3 ProjectVector(float3 a, float3 normalized)
{
    return normalized * dot(a, normalized);
}

float3 RemoveForce(float3 a, float3 b)
{
    float doted = dot(a, b);
    float lengthSq = dot(b, b);
    
    if (lengthSq == 0.0f)
    {
        return a;
    }
    
    return -b * (doted / lengthSq);
}

int GetCellIndex(int3 cell)
{
    return cell.x + cell.y * width + cell.z * width * height;
}

static const int checkCellCount = 26;
static const int3 nearCellIndex[checkCellCount] =
{
    { -1, -1, -1 },
    { -1, -1, 0 },
    { -1, -1, 1 },
    { -1, 0, -1 },
    { -1, 0, 0 },
    { -1, 0, 1 },
    { -1, 1, -1 },
    { -1, 1, 0 },
    { -1, 1, 1 },
    { 0, -1, -1 },
    { 0, -1, 0 },
    { 0, -1, 1 },
    { 0, 0, -1 },
    { 0, 0, 1 },
    { 0, 1, -1 },
    { 0, 1, 0 },
    { 0, 1, 1 },
    { 1, -1, -1 },
    { 1, -1, 0 },
    { 1, -1, 1 },
    { 1, 0, -1 },
    { 1, 0, 0 },
    { 1, 0, 1 },
    { 1, 1, -1 },
    { 1, 1, 0 },
    { 1, 1, 1 }
};

[numthreads(128, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;
    if (index >= particleCount)
    {
        return;
    }
    
    int i, j;
    //オブジェクトの位置を詳細に見るために、粒子の半径より少し大きめの距離を設定
    float collisionDistance = particleScale * 1.5f;
    float3 position = positions[index];
    float3 velocity = float3(0, 0, 0);
    float3 prevVelocity = velocities[index];
    float3 local = position - gridOrigin;
    
    int3 cell;
    cell.x = (int) (local.x / scale);
    cell.y = (int) (local.y / scale);
    cell.z = (int) (local.z / scale);
    
    int3 checkCell[checkCellCount + 1];
    checkCell[0] = cell;
    for (i = 0; i < checkCellCount; ++i)
    {
        checkCell[i + 1] = cell + nearCellIndex[i];
    }
    
    for (i = 0; i < checkCellCount + 1; ++i)
    {
        if (checkCell[i].x < 0 || checkCell[i].x >= width || checkCell[i].y < 0 || checkCell[i].y >= height || checkCell[i].z < 0 || checkCell[i].z >= depth)
        {
            continue;
        }
    
        int cellIndex = GetCellIndex(checkCell[i]);
        int count = particleCountInCell[cellIndex];
        int beginIndex = beginIndexForCell[cellIndex];
        
        for (j = 0; j < count; ++j)
        {
            int otherIndex = particleIndexForCell[beginIndex + j];
        
            if (otherIndex == index)
            {
                continue;
            }
        
            float3 diff = positions[otherIndex] - position;
            float dist = length(diff);
        
            if (dist < collisionDistance && dist != 0)
            {
                float3 n = normalize(diff);
                position -= n * (collisionDistance - dist) * 0.5f;
            }
        }
    }
    
    if (position.y < 0.0f)
    {
        velocity.y = 0.0f;
        //地面との摩擦
        velocity *= 0.75f;
    }
    
    position += velocity * 0.01666666f;
    
    if (position.y < 0.0f)
    {
        position.y = 0.0f;
        velocity.y = 0.0f;
    }
    
    velocity += prevVelocity;
    
    outputPositions[index] = position;
    velocities[index] = velocity;
}
