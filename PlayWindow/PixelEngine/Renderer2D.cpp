#include "Renderer2D.h"
#include "GameObject.h"
#include "Transform.h"
#include "PixelEngineAPI.h"
#include "PixelGraphicsAPI.h"
#include "PixelEngine.h"
#include "ResourceManager.h"
#include "LuaManager.h"
#include "BindManager.h"

extern PixelEngine* Engine;
Renderer2D::Renderer2D() :
	transform(nullptr),
	rendering(nullptr),
	textureName(""),
	textureID(-1)
{
	rendering = GetRenderingData();
	rendering->Type = QUAD;
}

Renderer2D::~Renderer2D()
{
	DeleteRenderingData(rendering);
}

void Renderer2D::Start()
{
	transform = targetObject->GetModule<Transform>();
}

void Renderer2D::LastUpdate()
{
	//rendering->changeTransform = false;
	if (transform == nullptr) return;
	Matrix m = transform->GetWorldMatrix();
	for (int i = 0; i < 16; i++)
	{
		rendering->World[i] = m._m[i];
	}
	//rendering->changeTransform = true;
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

std::string Renderer2D::RegisterLua()
{
	auto state = GetLuaState();
	state->new_usertype<Renderer2D>("Renderer2D", sol::base_classes, sol::bases<Module, PixelObject>(),
		"SetTexture", [](Renderer2D& obj, std::string name) {obj.SetTexture(name); }
	);
	
	std::string main = "";
	main += BindManager::ExportLuaAPIHeader<Renderer2D>();
	main += BindManager::ExportLuaAPIFromFunc("SetTexture",&Renderer2D::SetTexture,"string");
	return main;
}
