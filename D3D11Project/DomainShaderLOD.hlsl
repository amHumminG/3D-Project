cbuffer Camera : register(b0)
{
    float4x4 vpMatrix;
}

struct DomainShaderOutput
{
    float4 position : SV_POSITION;
    float3 worldPos : WORLD_POS;
    float3 normal : NORMAL;
    float2 uv : UV;
};

struct HullShaderOutput
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

#define NUM_CONTROL_POINTS 3



[domain("tri")]
DomainShaderOutput main
(
    HS_CONSTANT_DATA_OUTPUT input, float3 barycentric : SV_DomainLocation,
    const OutputPatch<HullShaderOutput, NUM_CONTROL_POINTS> patch
)
{
    // PHONG TESSELATION
    // 1. Compute the linear tessellation ( Done in the hull shader )
    // 2. Project the resulting point orthogonally onto the three tangent planes defined by the triangle vertices
    // 3. Compute the barycentric interpolation of these three projections
     
    
    float3 p0 = patch[0].worldPos;
    float3 p1 = patch[1].worldPos;
    float3 p2 = patch[2].worldPos;
    
    float3 n0 = normalize(patch[0].normal);
    float3 n1 = normalize(patch[1].normal);
    float3 n2 = normalize(patch[2].normal);
    
    // Compute the transformed position ( genereated by the hull shader )
    float3 p = mul(barycentric, float3x3(p0, p1, p2));
    
    // project p onto the plane represented by the points (p0, p1, p2) and the normals (n0, n1, n2)
    float3 q0 = p - dot((p - p0), n0) * n0;
    float3 q1 = p - dot((p - p1), n1) * n1;
    float3 q2 = p - dot((p - p2), n2) * n2;
    
    // Compute the barycentric interpolation of these three projections (q0, q1, q2)
    float3 newPos = mul(barycentric, float3x3(q0, q1, q2));
    
    float3 newNormal = normalize(mul(barycentric, float3x3(n0, n1, n2)));
    
    DomainShaderOutput output;
    output.worldPos = newPos;
    output.position = mul(float4(output.worldPos, 1.0f), vpMatrix);
    output.normal = newNormal;
    output.uv = mul(barycentric, float3x2(float2(patch[0].uv), float2(patch[1].uv), float2(patch[2].uv)));
    return output;
}