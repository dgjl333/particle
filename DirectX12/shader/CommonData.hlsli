struct vertData
{
    float4 position : POSITION;
    float2 uv : TEXCOORD;
};


struct v2f
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
};