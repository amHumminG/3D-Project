// Other need data such as constant buffers go here

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

RWStructuredBuffer<Particle> Particles : register(u0);

[numthreads(32, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    Particle gettingProcessed = Particles[DTid.x];
    
    float gravity = 0.01f;
    float windSpeed = 0.003f;
    
    if (gettingProcessed.position.y < 0)
    {
        // Reset yPos
        gettingProcessed.position = float3(gettingProcessed.startPos.x, gettingProcessed.startPos.y, gettingProcessed.startPos.z);
    }
    

    // snowflakes are falling
    gettingProcessed.position.y -= gravity;
    
    // modify x and z positions to simulate snowflakes gently moving in the wind
    gettingProcessed.position.x += windSpeed;
    gettingProcessed.position.z += windSpeed;
    
    Particles[DTid.x] = gettingProcessed;
}