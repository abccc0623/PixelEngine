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
	rendering(),
	textureName(""),
	textureID(-1)
{
	rendering.Type = RENDER_TYPE::QUAD;
}

Renderer2D::~Renderer2D()
{

}

void Renderer2D::LastUpdate()
{
	if (rendering.sprite.isAnimation)
	{
		nowFrameTime += GetDeltaTime() * animationSpeed;
		if (nowFrameTime >= oneFrameTime)
		{
			if (rendering.sprite.FramesIndex >= rendering.sprite.MaxFramesX)
			{
				rendering.sprite.FramesIndex = 0;
			}
			else
			{
				rendering.sprite.FramesIndex++;
			}
			nowFrameTime -= oneFrameTime;
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

void Renderer2D::SetAnimation(int MaxFramesX, int MaxFramesY, float speed)
{
	rendering.sprite.isAnimation = true;
	rendering.sprite.FramesIndex = 0;
	rendering.sprite.MaxFramesX = MaxFramesX;
	rendering.sprite.MaxFramesY = MaxFramesY;
	oneFrameTime = 1.0f / rendering.sprite.MaxFramesX;
	nowFrameTime = 0.0f;
	animationSpeed = speed;
}
