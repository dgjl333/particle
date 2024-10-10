#include "CommonData.hlsli"

v2f vert(vertData i)
{
    v2f o;
    o.position = i.position;
    o.uv = i.uv;
    return o;
}


Texture2D<float4> tex : register(t0);
SamplerState smp : register(s0);


float4 frag(v2f i) : SV_TARGET
{   
    //return float4(i.uv, 0, 1);
    return tex.Sample(smp, i.uv);
}