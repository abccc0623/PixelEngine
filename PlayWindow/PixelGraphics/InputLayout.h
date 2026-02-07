#pragma once
#include "DirectXMath.h"
struct StaticVertex
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT2 UV;
	DirectX::XMFLOAT3 Nomal;
	DirectX::XMFLOAT3 Tanget;
};

struct SkinningVertex
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT2 UV;
	DirectX::XMFLOAT3 Nomal;
	DirectX::XMFLOAT3 Tanget;

	DirectX::XMFLOAT4	BoneWeights;
	DirectX::XMINT4		BoneIndex;
};

struct DebugVertex
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT3 Color;
};