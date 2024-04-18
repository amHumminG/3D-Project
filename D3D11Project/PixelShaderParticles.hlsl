struct PixelShaderInput
{
    float4 worldPosition : SV_POSITION;
    //float3 velocity : VELOCITY;
    //float3 color : COLOR;
    //float size : SIZE;
    //float age : AGE;
    //float lifetime : LIFETIME;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
    float4 color = float4(1.0f, 1.0f, 1.0f, 1.0f);
    return color;
}