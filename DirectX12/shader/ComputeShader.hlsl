#include "CommonInput.hlsli"
#include "ParticleData.hlsli"

struct Particle
{
    float2 position;
    float2 velocity;
};

RWStructuredBuffer<Particle> particles : register(u1);

[numthreads(32, 1, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;

    Particle p = particles[index];
    
    p.velocity += float2(0, -1) * _DeltaTime * 100;
    p.position += p.velocity * _DeltaTime;
    
    p.position.x = clamp(p.position.x, s_leftBound + s_size * 0.5, s_rightBound - s_size * 0.5);
    p.position.y = clamp(p.position.y, s_lowerBound + s_size * 0.5, s_upperBound - s_size * 0.5);


    particles[index] = p;
}
