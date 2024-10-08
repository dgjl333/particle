#include "CommonData.hlsli"

float4 frag(v2f i) : SV_TARGET
{
    return float4(i.uv, 1, 1);
}