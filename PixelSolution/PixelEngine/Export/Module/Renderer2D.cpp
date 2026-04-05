#include "pch.h"
#include "Renderer2D.h"
#include "Core/GameObject.h"
#include "Module/Transform.h"
#include "PixelEngineAPI.h"
#include "PixelGraphicsAPI.h"
#include "PixelEngine.h"
#include "ResourceManager.h"
#include "LuaManager.h"
#include "BindManager.h"

extern PixelEngine* Engine;
Renderer2D::Renderer2D() :
	rendering(nullptr),
	textureName(""),
	textureID(-1)
{
	rendering = GetRenderingData();
	rendering->Type = QUAD;
	rendering->mesh.offsetX = 1;
	rendering->mesh.offsetY = 1;
	rendering->mesh.tilingX = 1;
	rendering->mesh.tilingY = 1;
	rendering->mesh.isSpriteSheet = false;
	rendering->mesh.frameCount	  = 1;
}

Renderer2D::~Renderer2D()
{
	DeleteRenderingData(rendering);
}


void Renderer2D::LastUpdate()
{
	testtime += GetDeltaTime();
	if (testtime >= 0.1f)
	{
		// 1. 현재 프레임 인덱스를 관리하는 변수를 별도로 두는 것이 좋습니다.
		// 만약 offsetX를 직접 써야 한다면 아래와 같이 계산하세요.
		static int frameIndex = 0;
		frameIndex = (frameIndex + 1) % 17; // 0 ~ 16 사이를 순환

		// 2. 오프셋은 (현재 인덱스 * 한 칸의 크기) 입니다.
		rendering->mesh.offsetX = (float)frameIndex * (1.0f / 17.0f);

		testtime = 0;
	}
	rendering->mesh.tilingX = 1.0f / 17.0f;

	rendering->mesh.tilingX = (1 / 17.0f);
	if (transform == nullptr) return;
	PMatrix m = transform->GetWorldMatrix();
	for (int i = 0; i < 16; i++)
	{
		rendering->World[i] = m._m[i];
	}
}

void Renderer2D::SetTexture(const std::string& name)
{
	textureName = name;
	textureID = Engine->GetResourceID(RESOURCE_TYPE::TEXTURE, textureName);
	if (rendering != nullptr)
	{
		rendering->mesh.texture_key = Engine->GetResourceID(RESOURCE_TYPE::TEXTURE, textureName);
		ChangeRenderingData(rendering);
	}
}
