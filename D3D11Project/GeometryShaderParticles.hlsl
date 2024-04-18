cbuffer CameraPosBuffer : register (b0)
{
    float3 cameraPosition;
};

cbuffer CameraVpBuffer : register(b1)
{
    float4x4 vpMatrix;
}

struct GeometryShaderOutput
{
    float4 pos : SV_POSITION;
};

[maxvertexcount(6)]
void main
(
    point float3 input[1] : POSITION,
    inout TriangleStream<GeometryShaderOutput> output
)
{
    // Calculate front vector using input and camera position
    // Use front vector along with defaulted up vector to calculate right vector
    // Use front vector along with right vector to calcualte actual up vector
    // Scale right and up vectors by size of particle
    
    float3 frontVector = normalize(input[0] - cameraPosition);
    float3 upVector = float3(0.0f, 1.0f, 0.0f);
    float3 rightVector = cross(frontVector, upVector);
    upVector = cross(rightVector, frontVector);

    float particleSize = 0.05f;
    rightVector *= particleSize;
    upVector *= particleSize;
    
    GeometryShaderOutput toAppend;
    
    // TRIANGLE 1
    toAppend.pos = mul(float4(input[0] - rightVector + upVector, 1.0f), vpMatrix); // Top left
    output.Append(toAppend);
    toAppend.pos = mul(float4(input[0] - rightVector - upVector, 1.0f), vpMatrix); //  Bottom left
    output.Append(toAppend);
    toAppend.pos = mul(float4(input[0] + rightVector - upVector, 1.0f), vpMatrix); // Bottom right
    output.Append(toAppend);
    output.RestartStrip();
    
    // TRIANGLE 2
    toAppend.pos = mul(float4(input[0] - rightVector + upVector, 1.0f), vpMatrix); // Top left
    output.Append(toAppend);
    toAppend.pos = mul(float4(input[0] + rightVector - upVector, 1.0f), vpMatrix); // Bottom right
    output.Append(toAppend);
    toAppend.pos = mul(float4(input[0] + rightVector + upVector, 1.0f), vpMatrix); // Top right
    output.Append(toAppend);
    output.RestartStrip();
}