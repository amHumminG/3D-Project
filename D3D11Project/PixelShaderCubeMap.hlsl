struct PixelShaderInput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
    float3 worldPosition : WORLD_POSITION;
};

struct PixelShaderOutput
{
    float4 position : SV_TARGET0;
    float4 color : SV_TARGET1;
    float4 normal : SV_TARGET2;
    float4 material : SV_Target3;
};

TextureCube TexCube : register(t0);
SamplerState Sampler : register(s0);

cbuffer materialbuffer : register(b0)
{
    float ambientk;
    float diffusek;
    float speculark;
    float shininess;
};

cbuffer cameraBuffer : register(b1)
{
    float3 cameraPos;
}


PixelShaderOutput main(PixelShaderInput input)
{
    float3 normal = normalize(input.normal);
    float3 incomingView = normalize(input.worldPosition - cameraPos);
    float3 reflectedView = (reflect(incomingView, input.normal));
    
    float4 textureColor = TexCube.Sample(Sampler, reflectedView);
    

    PixelShaderOutput output;
    output.position = float4(input.worldPosition, input.uv.x);
    output.color = textureColor;
    output.normal = float4(input.normal, input.uv.y);
    output.material = float4(ambientk, diffusek, speculark, shininess);
    return output;
}