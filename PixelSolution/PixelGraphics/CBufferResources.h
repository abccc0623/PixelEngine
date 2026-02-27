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
	DirectX::SimpleMath::Matrix TexMatrix;
};

struct SkinningBuffer
{
	DirectX::SimpleMath::Matrix Skinning_world;
	DirectX::SimpleMath::Matrix Skinning_Tex;
	DirectX::SimpleMath::Matrix BoneOffset[96];
};