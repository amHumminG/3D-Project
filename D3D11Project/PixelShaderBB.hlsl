struct PixelShaderInput
{
    float4 position : SV_POSITION;
    float3 color : COLOR;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
    float4 color = float4(input.color, 1.0f);
    return color;
}