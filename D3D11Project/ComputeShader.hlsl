RWTexture2D<unorm float4> OutputTextureUAV : register(u0);

Texture2D<float4> positionGBuffer : register(t0);
Texture2D<float4> colorGbuffer : register(t1);
Texture2D<float4> normalGBuffer : register(t2);
Texture2D<float4> materialGbuffer : register(t3);

cbuffer CameraBuffer : register(b0)
{
    float3 CameraPosition;
};

cbuffer LightInfoBuffer : register(b1)
{
    float numLights;
    float maxDistance; // the distance at which the light intensity is 0
    
    bool lightsOn;
};

struct LightBuffer
{
    matrix vpMatrix;
    float3 lightColor;
    float3 lightDirection;
    float lightAngle;
    float3 lightPosition;
};

StructuredBuffer<LightBuffer> lightBuffers : register(t4);
Texture2DArray<float> shadowMaps : register(t5);
sampler shadowSampler : register(s0);

#define SHADOW_EPSILON 0.0001f // to prevent self-shadowing

[numthreads(20, 20, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    int3 texLocation = int3(DTid.x, DTid.y, 0);
    
    float3 position = positionGBuffer[DTid.xy].xyz;
    float3 baseColor = colorGbuffer[DTid.xy].xyz;
    float3 normal = normalize(normalGBuffer[DTid.xy].xyz);
    float2 uv = float2(positionGBuffer[DTid.xy].w, normalGBuffer[DTid.xy].w);
    
    float ambient = materialGbuffer[DTid.xy].x;
    float diffuse = materialGbuffer[DTid.xy].y;
    float specular = materialGbuffer[DTid.xy].z;
    float shininess = materialGbuffer[DTid.xy].w;
    
    // is it necessary to store the uv coordinates in the gBuffer? if so, shouldnt we sample the texture here as well?
    
    //----------ambient component calculation-----------
    float3 ambientColor = baseColor * ambient;
    float3 diffuseColorFinal = float3(0.0f, 0.0f, 0.0f);
    float3 specularColorFinal = float3(0.0f, 0.0f, 0.0f);
    
    if (lightsOn)
    {
        // calculate lighting for each light
        for (int i = 0; i < numLights; i++)
        {
            // ---------- Shadow mapping calculations -----------
        
            float4 lightPos = mul(float4(position, 1.0f), lightBuffers[i].vpMatrix);            // to clip space
            float4 NDCpos = lightPos / lightPos.w;                                              // to NDC space
            float3 shadowMapUV = float3(NDCpos.x * 0.5f + 0.5f, NDCpos.y * -0.5f + 0.5f, i);    // to texture space
        
            float fragmentDepth = NDCpos.z - SHADOW_EPSILON;
            float sampleDepth = shadowMaps.SampleLevel(shadowSampler, shadowMapUV, 0.0f).r;
        
            bool isLit = fragmentDepth < sampleDepth;
        
            if (isLit)
            {
                // determine if the fragment is within the light cone
                float cutoffAngle = lightBuffers[i].lightAngle;
                float3 lightDirection = normalize(lightBuffers[i].lightDirection);
                float3 lightToFragment = normalize(position - lightBuffers[i].lightPosition);
                float cosAlfa = dot(lightDirection, lightToFragment);
                float cosCutoff = cos(cutoffAngle);
                if (cosAlfa > cosCutoff)
                {
                    // lighting calculations
            
                    float3 lightVector = lightBuffers[i].lightPosition - position;
                    float distanceToLight = length(lightVector);
                    lightVector = normalize(lightVector);
            
            
                    // --------- Light intensity calculations -----------
                    // light intesity based on how close fragment is to center of light cone (between 0 and 1)
            
                    //float maxDistance = 1000.0f; // the distance at which the light intensity is 0
            
                    float surfaceIntensity = 1 - (1 - cosAlfa) / (1 - cosCutoff);
                    float distanceIntensity = 1 - (distanceToLight / maxDistance);
                    float lightIntensity = surfaceIntensity * distanceIntensity;
            
                    // ---------- Diffuse component calculation -----------
                    normal = normalize(normal);
                    float irradianceFactor = max(0.0f, dot(normal, lightVector));
    
                    float diffuseComponent = diffuse * irradianceFactor;
                    float3 diffuseColor = lightBuffers[i].lightColor * diffuseComponent * lightIntensity;
            
                    diffuseColorFinal += diffuseColor;
            
                    // ---------- Specular component calculations -----------
                    float3 cameraVector = normalize(CameraPosition - position.xyz);
                    float3 reflectionVector = normalize(reflect(-lightVector, normal));
                    float reflectionFactor = max(0.0f, dot(reflectionVector, cameraVector));

                    float specularComponent = specular * pow(reflectionFactor, shininess);
                    float3 specularColor = lightBuffers[i].lightColor * specularComponent * lightIntensity;
                    specularColorFinal += specularColor;
                }
            }
        }
    }
    
    // ---------- Final color calculations -----------
    float4 finalColor = float4((ambientColor + diffuseColorFinal + specularColorFinal), 1.0f);
    

    OutputTextureUAV[DTid.xy] = finalColor;
}