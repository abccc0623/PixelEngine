#include "BoxCollision2D.h"
#include "PixelEngine.h"
#include "GameObject.h"
#include "Transform.h"
#include "CollisionManager.h"
#include "PixelGraphicsAPI.h"
#include "RenderringData.h"
#include <iostream>
extern PixelEngine* Engine;
BoxCollision2D::BoxCollision2D():
	collision(nullptr)
{
	rendering = GetRenderingData();
	rendering->Type = BOX2D;
	rendering->collision.offsetX = 0;
	rendering->collision.offsetY = 0;
	rendering->collision.sizeX = 0.5f;
	rendering->collision.sizeY = 0.5f;
}

BoxCollision2D::~BoxCollision2D()
{
	DeleteRenderingData(rendering);
}

void BoxCollision2D::Awake()
{
	collision = Engine->GetFactory<CollisionManager>();
}

void BoxCollision2D::Start()
{
	transform = targetObject->GetModule<Transform>();
}

void BoxCollision2D::PhysicsUpdate()
{

}

void BoxCollision2D::SetRect(float offsetX, float offsetY, float sizeX, float sizeY)
{
	rendering->collision.offsetX = offsetX;
	rendering->collision.offsetY = offsetY;
	rendering->collision.sizeX = sizeX;
	rendering->collision.sizeY = sizeY;
}

void BoxCollision2D::RegisterLua()
{
	//auto state = GetLuaState();
	//state->new_usertype<BoxCollision2D>("BoxCollision2D", sol::base_classes, sol::bases<Module, PixelObject>(),
	//	"SetRect", [](BoxCollision2D& obj, float offsetX, float offsetY, float sizeX, float sizeY) {obj.SetRect(offsetX, offsetY, sizeX, sizeY); }
	//);
	//
	//std::vector<std::string> functionName = std::vector<std::string>();
	//functionName.push_back("function BoxCollision2D.SetRect(...)");
	//AddLuaAPI("BoxCollision2D", functionName);

}


void BoxCollision2D::LastUpdate()
{
	Matrix m = transform->GetWorldMatrix();
	Matrix scale;
	Matrix position;
	scale = scale.CreateScale({ rendering->collision.sizeX,rendering->collision.sizeY,1});
	position = position.CreateTranslation({ rendering->collision.offsetX,rendering->collision.offsetY,1 });
	m = scale* position* m;
	for (int i = 0; i < 16; i++)
	{
		rendering->World[i] = m._m[i];
	}
}