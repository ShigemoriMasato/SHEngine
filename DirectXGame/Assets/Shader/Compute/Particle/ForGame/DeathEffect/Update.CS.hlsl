cbuffer ParticleCount : register(b0)
{
    uint particleCount;
};

cbuffer Config : register(b2)
{
    float deathTime;
    float3 gravity;
    float floor;
    float attenuation;

    uint isCharge;
    float scatY;
    float3 playerPos;
    float targetRadius;
    float lerpTime;


    uint seed;
}

cbuffer DeltaTime : register(b1)
{
    float deltaTime;
};

StructuredBuffer<uint> indexList : register(t0);

RWStructuredBuffer<float3> positions : register(u0);
RWStructuredBuffer<float3> rotations : register(u1);
RWStructuredBuffer<float> scales : register(u2);
RWStructuredBuffer<float16_t4> color : register(u3);
RWStructuredBuffer<float16_t3> velocity : register(u4);
RWStructuredBuffer<float16_t3> revolutionVelocity : register(u5);
RWStructuredBuffer<uint16_t> states : register(u6);
RWStructuredBuffer<float> timers : register(u7);

#define INVALID 0
#define DEATH 1
#define SETTING 2
#define SPREAD 3

uint Hash(uint x)
{
    x ^= x >> 16;
    x *= 0x7feb352d;
    x ^= x >> 15;
    x *= 0x846ca68b;
    x ^= x >> 16;
    return x;
}

float randf(inout uint state)
{
    state = Hash(state);
    float value = state / 4294967296.0;
    return value * 2 - 1;
}

[numthreads(128, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    int index = DTid.x;
    if (index >= particleCount)
    {
        return;
    }
    
    uint16_t state = states[index];
    float3 pos = position[index];
    float3 vel = velocity[index];
    float timer = timers[index];

    if (state == INVALID)
    {
        return;
    }
    //死亡エフェクト
    else if(state == DEATH)
    {
        timer += deltaTime;

        vel *= attenuation;
        vel += gravity * deltaTime;
        pos += vel * deltaTime;

        //反発(反発係数は適当)
        if(pos.y < floor)
        {
            pos.y = floor;
            vel.y *= -0.5f;
        }

        if(timer >= deathTime)
        {
            timer = 0.0f;
            ++state;

            //velocityを適当に決める。
            float3 tmp;
            tmp.x = randf(seed);
            tmp.y = randf(seed);
            tmp.z = randf(seed);

            tmp = normalize(tmp);

            tmp.y *= scatY;
        }

    }
    //プレイヤー周りで公転する
    else if(state == SETTING)
    {
        timer += deltaTime;

        float3 revVel = revolutionVelocity[index];
        float3 diff = playerPos - pos;
        float dist = length(diff);
        float3 dir = normalize(diff);
        
        timer = clamp(timer, 0.0f, lerpTime);
        vel = lerp(revVel, dir * , timer / lerpTime);
    }

    position[index] = pos;
    velocity[index] = vel;
    revolutionVelocity[index] = revVel;
    timers[index] = timer;
    
}