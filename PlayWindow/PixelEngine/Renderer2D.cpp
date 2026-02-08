#include "Renderer2D.h"
#include "GameObject.h"
#include "Transform.h"
#include "PixelEngineAPI.h"
#include "PixelGraphicsAPI.h"
#include "PixelEngine.h"
#include "ResourceManager.h"

extern PixelEngine* Engine;
Renderer2D::Renderer2D():
	transform(nullptr),
	rendering(nullptr),
	textureName(""),
	textureID(-1)
{
	rendering = GetRenderingData();
	rendering->Type = QUAD;
	rendering->Texture_ID = textureID;
}

Renderer2D::~Renderer2D()
{
	DeleteRenderingData(rendering);
}

void Renderer2D::Awake()
{

}

void Renderer2D::Start()
{
	transform = targetObject->GetModule<Transform>();
}

void Renderer2D::Update()
{
	
}

void Renderer2D::LastUpdate()
{
	Matrix m = transform->GetWorldMatrix();
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
		rendering->Texture_ID = Engine->GetResourceID(RESOURCE_TYPE::TEXTURE,textureName);
	}
}

void Renderer2D::RegisterLua()
{
	auto lua = GetLua();
	lua->new_usertype<Renderer2D>("Renderer2D",sol::base_classes, sol::bases<Module, BaseModule>(),
		"SetTexture", [](Renderer2D& obj, std::string name) {obj.SetTexture(name); }
	);
}
