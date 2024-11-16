#include "CommonStructure.hlsli"
#include "CommonInput.hlsli"

v2f vert(vertData i)
{
    v2f o;
    o.position = mul(_ProjectionMatrix, i.position);
    o.uv = i.uv;
    return o;
}


Texture2D<float4> tex : register(t2);
SamplerState smp : register(s0);


float4 frag(v2f i) : SV_TARGET
{   
    return tex.Sample(smp, i.uv);
}