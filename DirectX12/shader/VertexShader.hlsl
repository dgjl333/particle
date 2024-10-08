#include "CommonData.hlsli"

v2f vert(vertData i)
{
    v2f o;
    o.position = i.position;
    o.uv = i.uv;
    return o;
}