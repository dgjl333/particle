#pragma once

#include <DirectXMath.h>

using float4 = DirectX::XMFLOAT4;
using float3 = DirectX::XMFLOAT3;
using float2 = DirectX::XMFLOAT2;

struct Vertex
{
	float3 position;
	float2 uv;
};