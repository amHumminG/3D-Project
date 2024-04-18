struct PixelShaderInput
{
    float4 position : SV_POSITION;
    float3 worldPos : WORLD_POS;
    float3 normal : NORMAL;
    float2 uv : UV;
};

struct PixelShaderOutput
{
    float4 position : SV_TARGET0;
    float4 color : SV_TARGET1;
    float4 normal : SV_TARGET2;
    float4 material : SV_TARGET3;
};

Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

cbuffer materialbuffer : register(b0)
{
    float ambientk;
    float diffusek;
    float speculark;
    float shininess;
};


PixelShaderOutput main(PixelShaderInput input)
{
    float3 textureColor;
    textureColor = Texture.Sample(Sampler, input.uv);
    
    float3 normal = normalize(input.normal);
    
    PixelShaderOutput output;
    output.position = float4(input.worldPos, input.uv.x);
    output.color = float4(textureColor, 1.0f);
    output.normal = float4(input.normal, input.uv.y);
    output.material = float4(ambientk, diffusek, speculark, shininess);
    return output;
}