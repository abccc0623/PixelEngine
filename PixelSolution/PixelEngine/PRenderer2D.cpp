#include "pch.h"
#include "PRenderer2D.h"
#include "PixelEngineAPI.h"
#include "Registry.h"
#include "PixelEngine.h"
#include "ResourceManager.h"
extern PixelEngine* Engine;
void ECS::Renderer2D::Add(unsigned int id)
{
	auto registry = GetRegistry();
	registry->AddComponent<Renderer2DData>(id);
}

void ECS::Renderer2D::SetTexture(unsigned int id,const char* name)
{
	auto registry = GetRegistry();
	auto data = registry->Get<Renderer2DData>(id);
	data->renderingData.Type = RENDER_TYPE::QUAD;
	data->renderingData.sprite.TilingX = 1.0f;
	data->renderingData.sprite.TilingY = 1.0f;
	data->renderingData.sprite.OffsetX = 1.0f;
	data->renderingData.sprite.OffsetY = 1.0f;

	std::string textureName(name);
	auto textureID = Engine->GetResourceID(RESOURCE_TYPE::TEXTURE, textureName);
	data->renderingData.texture_key = textureID;

}

void ECS::Renderer2D::SetTextureOffset(unsigned int id,float OffsetX, float OffsetY)
{


}

void ECS::Renderer2D::AddTextureOffset(unsigned int id,float OffsetX, float OffsetY)
{


}
void ECS::Renderer2D::SetTextureTiling(unsigned int id,float TilingX, float TilingY)
{

}
void ECS::Renderer2D::AddTextureTiling(unsigned int id,float TilingX, float TilingY)
{

}