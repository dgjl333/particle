#include "CommonInput.hlsli"
#include "ParticleData.hlsli"

#define PI 3.1415926

struct vIn
{
    uint id : SV_VERTEXID;
};

struct v2g
{
    float4 position : POSITION;
    float2 velocity : TEXCOORD0;
};

struct g2f
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
    float2 velocity : TEXCOORD1;
    float2 screenPos : TEXCOORD2;
};

struct Particle
{
    float2 position;
    float2 velocity;
};

StructuredBuffer<Particle> particles : register(t2);

float Rand(float2 p)
{
    p = frac(p * float2(123.34, 345.45));
    p += dot(p, p + 34.345);
    return frac(p.x * p);
}

float2 ComputeScreenPos(float4 pos)
{
    float2 xy = (pos.xy + pos.w) * 0.5;
    return xy / pos.w;
}


v2g vert(vIn i)
{
    v2g o;
    o.position = float4(particles[i.id].position.xy, 0, 1);
    o.velocity = particles[i.id].velocity;
    return o;
}

[maxvertexcount(4)]
void geom(point v2g input[1], inout TriangleStream<g2f> stream)
{
    float4 pos = input[0].position;
    float2 velocity = input[0].velocity;
    g2f o;
    
    o.velocity = velocity;
    
    float4 lowerLeft = pos + float4(-s_size, -s_size, 0, 0);
    o.position = mul(_ProjectionMatrix, lowerLeft);
    o.screenPos = ComputeScreenPos(o.position);
    o.uv = float2(0, 0);
    stream.Append(o);
    
    float4 lowerRight = pos + float4(s_size, -s_size, 0, 0);
    o.position = mul(_ProjectionMatrix, lowerRight);
    o.screenPos = ComputeScreenPos(o.position);
    o.uv = float2(1, 0);
    stream.Append(o);
    
    float4 upperLeft = pos + float4(-s_size, s_size, 0, 0);
    o.position = mul(_ProjectionMatrix, upperLeft);
    o.screenPos = ComputeScreenPos(o.position);
    o.uv = float2(0, 1);
    stream.Append(o);
    
    float4 upperRight = pos + float4(s_size, s_size, 0, 0);
    o.position = mul(_ProjectionMatrix, upperRight);
    o.screenPos = ComputeScreenPos(o.position);
    o.uv = float2(1, 1);
    stream.Append(o);
    
    stream.RestartStrip();
}


float3 hsl2rgb(float3 c)
{
    float3 rgb = clamp(abs(fmod(c.x * 6.0 + float3(0.0, 4.0, 2.0), 6.0) - 3.0) - 1.0, 0.0, 1.0);

    return c.z + c.y * (rgb - 0.5) * (1.0 - abs(2.0 * c.z - 1.0));
}

float4 frag(g2f i) : SV_TARGET
{
    float2 uv = i.uv * 2 - 1;
    float dist = length(uv);
    
    float h = 0.5 * (sin(0.4 * (i.screenPos.x + i.screenPos.y) + _Time.y * 0.1 + _Seed * 2 * PI) + 1);
    float s = lerp(0.35, 0.75, smoothstep(15, 150, length(i.velocity)));
    
    float2 border = min(i.screenPos, 1 - i.screenPos);
    float l = lerp(0.0, 0.5, smoothstep(0.002, 0.015, min(border.x, border.y)));
    
    float3 rgb = hsl2rgb(float3(h, s, l));
    
    float clip = smoothstep(0.01, 1, dist);
    
    return lerp(float4(rgb, 1), 0, clip);
}