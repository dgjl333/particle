#include "CommonInput.hlsli"
#define PARTICLE_COUNT 100000

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

    if (index >= PARTICLE_COUNT)
        return;

    Particle p = particles[index];
    
    //p.velocity.xy += float2(0, -1) * _DeltaTime;
    //p.position.xy += p.velocity.xy * _DeltaTime;

    particles[index] = p;
}
