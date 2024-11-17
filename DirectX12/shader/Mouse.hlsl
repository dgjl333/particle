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


v2f vert(vIn i)
{
    v2f o;
    o.position = mul(_ProjectionMatrix, i.position);
    o.uv = i.uv;
    return o;
}

Texture2D<float4> tex : register(t2);
SamplerState smp : register(s0);

static float s_alpha = 0.8;
static float s_size = 0.75;
static float s_inner = 0.1;
static float s_thickness = 0.1;
static float s_border = 0.035;

float4 frag(v2f i) : SV_TARGET
{   
    //float3 color = tex.Sample(smp, i.uv);
    float3 color = 1;
    float2 uv = i.uv * 2 - 1;
    color = 0.0125 / dot(uv, uv);
   // float innerAlpha = lerp(0, s_alpha, smoothstep(s_inner, s_inner + s_border, s_size * length(uv)));
    //float outerAlpha = lerp(s_alpha, 0, smoothstep(s_inner + s_thickness, s_inner + s_thickness + s_border, s_size * length(uv)));
    return float4(color, color.x-0.2);
}