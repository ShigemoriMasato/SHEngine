struct Wave
{
    float2 direction;
    float amplitude;
    float wavelength;
    float speed;
    float3 pad;
};

cbuffer CBPerObject : register(b0)
{
    float4x4 world;
    float4x4 vp;
    float3 cameraPos;
    float time;
    int waveCount;
    float3 pad;
    Wave waves[8];
};

struct VSInput
{
    float4 pos : POSITION0;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL0;
};

struct VSOutput
{
    float4 posH : SV_POSITION;
    float3 worldPos : TEXCOORD0;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD1;
    float3 viewDir : TEXCOORD2;
};

float kFromWavelength(float wavelength)
{
    return 2.0 * 3.14159265 / wavelength;
}

VSOutput main(VSInput input)
{
    VSOutput o;
    float3 pos = mul(input.pos, world).xyz; // pos.y is baseline (0)
    float3 displaced = pos;
    float3 tangent = float3(1, 0, 0);
    float3 bitangent = float3(0, 0, 1);

    // accumulate displacement and partial derivatives for normal
    float3 dPos_dx = float3(1, 0, 0);
    float3 dPos_dz = float3(0, 0, 1);

    for (int i = 0; i < waveCount; ++i)
    {
        float2 D = normalize(waves[i].direction);
        float A = waves[i].amplitude;
        float L = waves[i].wavelength;
        float S = waves[i].speed;
        float k = kFromWavelength(L);
        float w = S * k; // angular frequency approximation

        float phase = dot(D, pos.xz) * k - w * time;
        float cosP = cos(phase);
        float sinP = sin(phase);

        // horizontal displacement
        displaced.x += D.x * (A * cosP);
        displaced.z += D.y * (A * cosP);
        displaced.y += A * sinP;

        // partial derivatives for normal
        // ∂p/∂x and ∂p/∂z contributions
        // from derivation:
        dPos_dx.x += -D.x * D.x * A * k * sinP;
        dPos_dx.y += D.x * A * k * cosP;
        dPos_dx.z += -D.x * D.y * A * k * sinP;

        dPos_dz.x += -D.y * D.x * A * k * sinP;
        dPos_dz.y += D.y * A * k * cosP;
        dPos_dz.z += -D.y * D.y * A * k * sinP;
    }

    // compute normal from cross product of partials
    float3 normal = normalize(cross(dPos_dz, dPos_dx)); // note order
    pos = displaced;
    o.worldPos = pos;
    o.normal = mul((float3x3) world, normal); // transform normal correctly
    o.uv = input.uv;
    float4 viewPos = mul(float4(pos, 1), vp);
    o.viewDir = normalize(cameraPos - pos);

    o.posH = mul(float4(pos, 1), vp);
    return o;
}
