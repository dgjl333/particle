#include "CommonInput.hlsli"

struct vIn
{
    float4 position : POSITION;
    float2 uv : TEXCOORD;
};

struct v2f
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
};

float Rand(float2 p)
{
    p = frac(p * float2(123.34, 345.45));
    p += dot(p, p + 34.345);
    return frac(p.x * p);
}

cbuffer Data : register(b2)
{
    float _Flash;
};

v2f vert(vIn i)
{
    v2f o;
    o.position = mul(_ProjectionMatrix, i.position);
    o.uv = i.uv;
    return o;
}

float4 frag(v2f i) : SV_TARGET
{   
    float3 color = 1;
    float2 uv = i.uv * 2 - 1;
    color = 0.0125 / pow(length(uv) + _Flash * 0.075 * 0.5 * (sin(_Time.z * 10) + 1), 2);
    return float4(color, max(0, color.x - 0.2));
}