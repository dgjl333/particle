#include "CommonInput.hlsli"
#include "ParticleData.hlsli"
#include "Noise.hlsli"

struct Particle
{
    float2 position;
    float2 velocity;
};

RWStructuredBuffer<Particle> particles : register(u1);

cbuffer Data : register(b2)
{
    float2 _MousePos;
    float _ForceStrength;
};

float2 curlNoise(float2 p)
{
    float2 grad = SimplexNoiseGrad(p).xy;
    return float2(-grad.y, grad.x);
}

[numthreads(32, 1, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;
    Particle p = particles[index];
    
    float t = _Time.y * 0.5;
    
    float2 offset = p.position - _MousePos;
    float2 constantForce = 1 / (pow(length(offset), 2) + 0.5) * normalize(offset);
    
    float2 curl = curlNoise(p.position * s_noiseScale + _Seed * 1000);
    p.velocity += curl * _DeltaTime * s_noiseStrength;
    p.velocity += constantForce * _DeltaTime * _ForceStrength * 10000000;
    p.velocity.y += 0.5 * (sin(t) + 0.2 * cos(t + cos(2 * t + 1)));

    float drag = s_viscosity * length(p.velocity) * _DeltaTime;
    p.velocity *= max(1 - drag, 0);
    
    p.position += p.velocity * _DeltaTime;
    
    float width = 2 / _ProjectionMatrix[0][0];
    float leftBound = s_size * 0.5;
    float rightBound = width - s_size * 0.5;
    
    if (p.position.x < leftBound || p.position.x > rightBound)
    {
        p.velocity.x *= -s_damping;
        p.position.x = clamp(p.position.x, leftBound, rightBound);
    }
    
    float height = 2 / _ProjectionMatrix[1][1];
    float lowerBound = s_size * 0.5;
    float upperBound = height - s_size * 0.5;
    
    if (p.position.y < lowerBound || p.position.y > upperBound)
    {
        p.velocity.y *= -s_damping;
        p.position.y = clamp(p.position.y, lowerBound, upperBound);
    }

    particles[index] = p;
}
