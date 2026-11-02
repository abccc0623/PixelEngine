#pragma once
#include "DirectXMath.h"
#include "SimpleMath.h"
//카메라 버퍼
struct CameraBuffer
{
	DirectX::SimpleMath::Matrix view;
	DirectX::SimpleMath::Matrix proj;
	DirectX::SimpleMath::Matrix view_proj;
};

//오브젝트 버퍼
struct ObjectBuffer
{
	DirectX::SimpleMath::Matrix world;
	DirectX::SimpleMath::Matrix wvp;
	DirectX::SimpleMath::Matrix TexMatrix;
	DirectX::SimpleMath::Vector4 Color;
	// x is the alpha-test threshold. UI/cutout objects retain the existing threshold.
	DirectX::SimpleMath::Vector4 AlphaSettings = DirectX::SimpleMath::Vector4(0.1f, 0.0f, 0.0f, 0.0f);
};
static_assert(sizeof(ObjectBuffer) == 224, "ObjectBuffer must match Static_vs/Static_ps HLSL layout");

struct SkinningBuffer
{
	DirectX::SimpleMath::Matrix Skinning_world;
	DirectX::SimpleMath::Matrix Skinning_Tex;
	DirectX::SimpleMath::Matrix BoneOffset[96];
};
