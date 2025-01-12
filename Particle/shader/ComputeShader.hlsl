#include "CommonInput.hlsli"
#include "ParticleData.hlsli"
#include "Noise.hlsli"

struct Particle
{
    float2 position;
    float2 velocity;
};

RWStructuredBuffer<Particle> particles : register(u1);

cbuffer Data : register(b1)
{
    float _Drag;
    float _CurlScale;
    float _CurlStrength;
    float _Turbulence;
    
    float2 _MousePos;
    float _ConstantForceStrength;
};

float2 ConstantForce(float2 p)
{
    float2 offset = p - _MousePos;
    float distance = length(offset);
    return 1 / (pow(distance, 2) + 1) * normalize(offset) * _ConstantForceStrength * 500000;
}

float2 CurlNoise(float2 p)
{
    float2 grad = SimplexNoiseGrad(p).xy;
    return float2(-grad.y, grad.x);
}

float mod(float a, float b)
{
    return a - b * floor(a / b);
}

[numthreads(PARTICLE_NUMTHREADS, 1, 1)]
void CSMain(uint3 id : SV_DispatchThreadID)
{
    uint index = id.x;
    Particle p = particles[index];
    
    float t = _Time.y * 0.5;
    
    float2 curl = CurlNoise(p.position * _CurlScale / 1000 + _Seed * 1000);
    p.velocity += curl * _DeltaTime * _CurlStrength * 10;
    p.velocity += ConstantForce(p.position) * _DeltaTime * 7.5;
    p.velocity.y += _CurlStrength * _Turbulence / 70 * (sin(t) + 0.2 * cos(t + cos(2 * t + 1)));

    float drag = _Drag / 100 * length(p.velocity) * _DeltaTime;
    p.velocity *= max(1 - drag, 0);
    
    p.position += p.velocity * _DeltaTime;
    
    float width = 2 / _ProjectionMatrix[0][0];
    float leftBound = s_size * 0.5;
    float rightBound = width - s_size * 0.5;
     
    float height = 2 / _ProjectionMatrix[1][1];
    float lowerBound = s_size * 0.5;
    float upperBound = height - s_size * 0.5;
    
    p.position.x = mod(p.position.x - leftBound, rightBound - leftBound) + leftBound;   
    p.position.y = mod(p.position.y - lowerBound, upperBound - lowerBound) + lowerBound;

    particles[index] = p;
}