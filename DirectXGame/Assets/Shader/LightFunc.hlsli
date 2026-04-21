
struct DirectionalLight
{
    float4 color;
    float3 direction;
    float intensity;
};

struct PointLight
{
    float4 color;
    float3 position;
    float intensity;
    float radius;
    float decay;
    float2 pad;
};

float PointLightAttenuation(float3 lightPos, float3 objPos, float radius, float decay, float intensity)
{
    float distance = length(lightPos - objPos);
    return pow(saturate(-distance / radius + 1.0), decay) * intensity;
}

float PointLightAttenuationSimply(float3 lightPos, float3 objPos, float intensity)
{
    float distance = length(objPos - lightPos);
    return (1.0f - (distance / 10.0f - intensity)) / 2.0f;
}

float4 HalfLambert(float3 normal, float4 color, float4 lightColor, float3 direction, float intensity)
{
    float NdotL = dot(normalize(normal), -direction);
    float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
    float4 outputColor = color * lightColor * cos * intensity;
    outputColor.w = color.w;
    return outputColor;
}

float4 LambertReflectance(float3 normal, float4 color, float4 lightColor, float3 direction, float intensity)
{
    float NdotL = dot(normalize(normal), -direction);
    float cos = saturate(dot(normalize(normal), -direction));
    float4 outputColor = color * lightColor * cos * intensity;
    outputColor.w = color.w;
    return outputColor;
}


float4 PhongReflection(float3 cameraPos, float3 objPos, float3 lightDir, float3 normal, float shininess, float4 objColor, float4 lightedColor, float4 lightColor)
{
    float3 toEye = normalize(cameraPos - objPos);
    
    float3 reflectLight = reflect(lightDir, normalize(normal));
    
    float RdotE = dot(reflectLight, toEye);
    float specular = pow(saturate(RdotE), shininess);
    
    float3 specularColor = objColor.rgb * specular * lightColor.rgb;
    
    return float4(lightedColor.rgb + specularColor, lightedColor.w);
}
