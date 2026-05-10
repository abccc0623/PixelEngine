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
#include "PRenderer2D.h"

extern PixelEngine* Engine;
Renderer2D::Renderer2D() :
	rendering(),
	textureName(""),
	textureID(-1)
{
	rendering.Type = RENDER_TYPE::QUAD;
	rendering.sprite.isShared = true;
	rendering.sprite.TilingX = 1.0f;
	rendering.sprite.TilingY = 1.0f;
	rendering.sprite.OffsetX = 1.0f;
	rendering.sprite.OffsetY = 1.0f;
}

Renderer2D::~Renderer2D()
{

}

void Renderer2D::LastUpdate()
{
	if (nowAnimationIndex != -1)
	{
		nowAnimation.nowFrameTime += GetDeltaTime() * nowAnimation.animationSpeed;
		if (nowAnimation.nowFrameTime >= nowAnimation.oneFrameTime)
		{
			// ⭐ 총 프레임 수(X * Y)를 계산해서 비교합니다.
			int totalFrames = nowAnimation.maxFramesX * nowAnimation.maxFramesY;

			if (nowAnimation.framesIndex >= totalFrames - 1)
			{
				nowAnimation.framesIndex = 0;
			}
			else
			{
				nowAnimation.framesIndex++;
			}
			nowAnimation.nowFrameTime -= nowAnimation.oneFrameTime;

			rendering.sprite.TilingX = 1.0f / nowAnimation.maxFramesX;
			rendering.sprite.TilingY = 1.0f / nowAnimation.maxFramesY;

			rendering.sprite.OffsetX = (nowAnimation.framesIndex % nowAnimation.maxFramesX) * rendering.sprite.TilingX;
			rendering.sprite.OffsetY = (nowAnimation.framesIndex / nowAnimation.maxFramesX) * rendering.sprite.TilingY;
		}
	}



	if (transform == nullptr) return;
	PMatrix m = transform->GetWorldMatrix();
	for (int i = 0; i < 16; i++)
	{
		rendering.World[i] = m._m[i];
	}
	SetRenderingData(rendering);
}

void Renderer2D::SetTexture(const std::string& name)
{
	textureID = Engine->GetResourceID(RESOURCE_TYPE::TEXTURE, name);
	rendering.texture_key = textureID;
}

void Renderer2D::SetMaterial(const std::string& name)
{
	materialID = Engine->GetResourceID(RESOURCE_TYPE::MATERIAL, name);
	rendering.material_key = materialID;
}


int Renderer2D::CreateAnimation(std::string textureName,int MaxFramesX, int MaxFramesY, float speed)
{
	AnimationData data;
	data.textureID = textureID = Engine->GetResourceID(RESOURCE_TYPE::TEXTURE,textureName);
	data.framesIndex = 0;
	data.maxFramesX = MaxFramesX;
	data.maxFramesY = MaxFramesY;
	data.oneFrameTime = 1.0f / MaxFramesX;
	data.nowFrameTime = 0.0f;
	data.animationSpeed = speed;
	int index = animationList.size();
	animationList.push_back(data);

	rendering.sprite.TilingX = 1.0f / MaxFramesX;
	rendering.sprite.TilingY = 1.0f / MaxFramesY;
	rendering.sprite.OffsetX = (data.framesIndex % MaxFramesX) * rendering.sprite.TilingX;
	rendering.sprite.OffsetY = (data.framesIndex / MaxFramesX) * rendering.sprite.TilingY;
	return index;
}

void Renderer2D::PlayAnimation(int index)
{
	//예외처리
	if (index > animationList.size() - 1)
	{
		PixelLog::Error("Index out of range");
		PixelLog::Error("Now Max range:" + std::to_string(animationList.size() - 1));
		return;
	}

	if (index == nowAnimationIndex) return;
	//이제 이 Renderer는 다른오브젝트와 개별적으로 움직인다.
	rendering.sprite.isShared = false;


	//애니메이션 초기셋팅
	nowAnimationIndex = index;
	nowAnimation = animationList[nowAnimationIndex];
	nowAnimation.framesIndex = 0;
	nowAnimation.nowFrameTime = 0;
	
	//애니메이션 텍스쳐변경
	textureID = nowAnimation.textureID;
	rendering.texture_key = textureID;

	rendering.sprite.TilingX = 1.0f / nowAnimation.maxFramesX;
	rendering.sprite.TilingY = 1.0f / nowAnimation.maxFramesY;
	rendering.sprite.OffsetX = (nowAnimation.framesIndex % nowAnimation.maxFramesX) * rendering.sprite.TilingX;
	rendering.sprite.OffsetY = (nowAnimation.framesIndex / nowAnimation.maxFramesX) * rendering.sprite.TilingY;
}



void Renderer2D::SetTextureOffset(float OffsetX, float OffsetY)
{
	AnimationClear();
	rendering.sprite.OffsetX = OffsetX;
	rendering.sprite.OffsetY = OffsetY;
}

void Renderer2D::AddTextureOffset(float OffsetX, float OffsetY)
{
	AnimationClear();
	//rendering.sprite.OffsetX
	//rendering.sprite.OffsetY
}

void Renderer2D::SetTextureTiling(float TilingX, float TilingY)
{
	AnimationClear();
	rendering.sprite.TilingX = TilingX;
	rendering.sprite.TilingY = TilingY;
}

void Renderer2D::AddTextureTiling(float TilingX, float TilingY)
{
	AnimationClear();
}

void Renderer2D::AnimationClear()
{
	nowAnimationIndex = -1;
	rendering.sprite.isShared = false;
}
