#include "CommonInput.hlsli"
#include "ParticleData.hlsli"

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
};

struct Particle
{
    float2 position;
    float2 velocity;
};

StructuredBuffer<Particle> particles : register(t2);

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
    
    float4 lowerLeft = pos + float4(-s_size, -s_size, 0, 0);
    o.position = mul(_ProjectionMatrix, lowerLeft);
    o.uv = float2(0, 0);
    o.velocity = velocity;
    stream.Append(o);
    
    float4 lowerRight = pos + float4(s_size, -s_size, 0, 0);
    o.position = mul(_ProjectionMatrix, lowerRight);
    o.uv = float2(1, 0);
    o.velocity = velocity;
    stream.Append(o);
    
    float4 upperLeft = pos + float4(-s_size, s_size, 0, 0);
    o.position = mul(_ProjectionMatrix, upperLeft);
    o.uv = float2(0, 1);
    o.velocity = velocity;
    stream.Append(o);
    
    float4 upperRight = pos + float4(s_size, s_size, 0, 0);
    o.position = mul(_ProjectionMatrix, upperRight);
    o.uv = float2(1, 1);
    o.velocity = velocity;
    stream.Append(o);
    
    stream.RestartStrip();
}


float4 frag(g2f i) : SV_TARGET
{
    float2 uv = i.uv * 2 - 1;
    float dist = length(uv);
    
    float4 hue = lerp(s_blue, s_purple, smoothstep(8, 35, length(i.velocity)));
    float4 color = lerp(hue, 0, smoothstep(0.1, 1, dist));
    
    return color;
}