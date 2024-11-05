#include "CommonInput.hlsli"
#include "ParticleData.hlsli"
#include "Noise.hlsli"

struct Particle
{
    float2 position;
    float2 velocity;
};

RWStructuredBuffer<Particle> particles : register(u1);

float2 curlNoise(float2 position)
{
    float2 grad = SimplexNoiseGrad(position).xy;
    return float2(-grad.y, grad.x);
}

[numthreads(32, 1, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;

    Particle p = particles[index];
    
    p.velocity += curlNoise(p.position * _Time.y) * _DeltaTime * s_strength;
    float drag =  s_dragFactor * length(p.velocity) * _DeltaTime;
    p.velocity *= max(1 - drag, 0);
    
    p.position += p.velocity * _DeltaTime;
    
    float leftBound = s_leftBound + s_size * 0.5;
    float rightBound = s_rightBound - s_size * 0.5;
    
    if (p.position.x < leftBound || p.position.x > rightBound)
    {
        p.velocity.x *= -s_damping;
        p.position.x = clamp(p.position.x, leftBound, rightBound);
    }
    
    float lowerBound = s_lowerBound + s_size * 0.5;
    float upperBound = s_upperBound - s_size * 0.5;
    
    if (p.position.y < lowerBound || p.position.y > upperBound)
    {
        p.velocity.y *= -s_damping;
        p.position.y = clamp(p.position.y, lowerBound, upperBound);
    }

    particles[index] = p;
}
