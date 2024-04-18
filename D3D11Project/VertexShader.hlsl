struct VertexShaderInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float3 uv : UV;
};

struct VertexShaderOutput
{
    float3 worldPos : WORLD_POS;
    float3 normal : NORMAL;
    float2 uv : UV;
};

cbuffer ObjectBuffer : register(b0)
{
    matrix WorldMatrix;
}

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
	
    float4 worldPosition = mul(float4(input.position, 1.0f), WorldMatrix);

    output.normal = normalize(mul(input.normal, (float3x3) WorldMatrix));
    output.uv = input.uv;
    output.worldPos = worldPosition;
	
    return output;
}