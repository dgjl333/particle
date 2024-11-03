#include "CommonStructure.hlsli"
#include "CommonInput.hlsli"


struct Matrix
{
    float4 _Color;
    float4x4 mat;
};
ConstantBuffer<Matrix> m : register(b2);


v2f vert(vertData i)
{
    v2f o;
    o.position = mul(_ProjectionMatrix, i.position);
    o.uv = i.uv;
    return o;
}


float4 frag(v2f i) : SV_TARGET
{
    return m._Color + 0.5 * (sin(_Time.y) + 1);
}