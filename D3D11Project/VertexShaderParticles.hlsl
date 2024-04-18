struct Particle
{
    float3 position;
    float padding;
    float3 startPos;
    float padding2;
    //float3 velocity;
    //float3 color;
    //float size;
    //float age;
    //float lifetime;
};

//struct VertexShaderOutput
//{
//    float3 position : POSITION;
//    float3 startPos : STARTPOS;
//};

StructuredBuffer<Particle> Particles : register(t0);

float3 main(uint vertexID : SV_VertexID) : POSITION
{
    return Particles[vertexID].position;
}