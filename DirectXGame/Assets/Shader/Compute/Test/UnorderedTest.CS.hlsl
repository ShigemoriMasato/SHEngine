RWTexture2D<float4> outputTexture : register(u0);

[numthreads(128, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    static const int textureWidth = 1280;
    static const int textureHeight = 720;

    int width = DTid.x % textureWidth;
    int height = DTid.x / textureWidth;

    if(width >= textureWidth || height >= textureHeight)
        return;
        
    //中心からの距離を求める
    float dist = distance(float2(width, height), float2(textureWidth / 2, textureHeight / 2));

    if(dist < 100){
        outputTexture[uint2(width, height)] = float4(1, 1, 1, 1); // Set the pixel to red
    }else{
        outputTexture[uint2(width, height)] = float4(0.85, 0.9, 0.95, 1); // Set the pixel to Pastel Blue
    }
}