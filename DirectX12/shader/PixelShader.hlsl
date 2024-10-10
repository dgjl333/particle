#include "CommonData.hlsli"

Texture2D<float4> tex : register(t0);
SamplerState smp : register(s0);

float4 frag(v2f i) : SV_TARGET
{   
    //return float4(i.uv, 0, 1);
    return tex.Sample(smp, i.uv);
}