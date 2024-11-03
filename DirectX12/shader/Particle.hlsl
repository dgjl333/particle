#include "CommonInput.hlsli"

struct vIn
{
    uint id : SV_VERTEXID;
};

struct v2f
{
    float4 position : SV_Position;
};

struct Particle
{
    float2 position;
    float2 velocity;
};

StructuredBuffer<Particle> particles : register(t2);

float N21(float2 p)
{
    p = frac(p * float2(123.34, 345.45));
    p += dot(p, p + 34.345);
    return frac(p.x * p);
}

v2f vert(vIn i)
{
    v2f o;
    float4 position = float4(particles[i.id].position.xy, 0, 1);
    o.position = mul(_ProjectionMatrix, position);
    return o;
}


float4 frag(v2f i) : SV_TARGET
{
    return 1;
}