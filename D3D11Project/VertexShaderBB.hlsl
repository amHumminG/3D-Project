struct VertexShaderInput
{
    float3 position : POSITION;
    float3 color : COLOR;
};

struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float3 color : COLOR;
};

cbuffer cameraBuffer : register(b0)
{
    matrix vpMatrix;
}

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
	
    output.position = mul(float4(input.position, 1.0f), vpMatrix);
    output.color = input.color;
	
    return output;
}