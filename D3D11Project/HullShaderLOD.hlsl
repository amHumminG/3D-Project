struct VertexShaderOutput
{
    float3 worldPos : WORLD_POS;
    float3 normal : NORMAL;
    float2 uv : UV;
};

struct HS_CONSTANT_DATA_OUTPUT
{
    float EdgeTessFactor[3] : SV_TessFactor;
    float InsideTessFactor : SV_InsideTessFactor;
};

cbuffer Camera : register (b0)
{
    float3 cameraPos;
}

#define NUM_CONTROL_POINTS 3

HS_CONSTANT_DATA_OUTPUT CalcHsPatchConstants(InputPatch<VertexShaderOutput, NUM_CONTROL_POINTS> ip)
{
    HS_CONSTANT_DATA_OUTPUT output;
    
    // tessFactor based on distance from camera
    float3 center = (ip[0].worldPos + ip[1].worldPos + ip[2].worldPos) / 3.0f;
    float3 toCamera = center - cameraPos;
    float distance = length(toCamera);
    uint tessFactor = 4;
    
    if (distance < 5)
        tessFactor = 4;
    else if (distance < 10)
        tessFactor = 3;
    else if (distance < 20)
        tessFactor = 2;
    else
        tessFactor = 1;
    
    output.EdgeTessFactor[0] = tessFactor;
    output.EdgeTessFactor[1] = tessFactor;
    output.EdgeTessFactor[2] = tessFactor;
    output.InsideTessFactor = tessFactor;
    
    return output;
}


struct HullShaderOutput
{
    float3 worldPos : WORLD_POS;
    float3 normal : NORMAL;
    float2 uv : UV;
};

[domain("tri")]
[partitioning("fractional_odd")] // fractional_even, fractional_odd, integer
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("CalcHsPatchConstants")]
HullShaderOutput main
(
    InputPatch<VertexShaderOutput, NUM_CONTROL_POINTS> ip,
    uint i : SV_OutputControlPointID
)
{
    HullShaderOutput output;
    
    output.worldPos = ip[i].worldPos;
    output.normal = ip[i].normal;
    output.uv = ip[i].uv;
    
    return output;
}
