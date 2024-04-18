struct VertexShaderInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float3 uv : UV;
};

// Defining the constant buffer
cbuffer matrixCbuffer : register(b0)
{
    matrix WorldMatrix;
    matrix viewProjectionMatrix;
}

float4 main(VertexShaderInput input) : SV_POSITION
{
    float4 worldPosition = mul(float4(input.position, 1.0f), WorldMatrix);
    float4 clipPosition = mul(worldPosition, viewProjectionMatrix);
	
    return clipPosition;
}