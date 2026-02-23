#include "BoxCollision2D.h"
#include "PixelEngine.h"
#include "GameObject.h"
#include "Transform.h"
#include "CollisionManager.h"
#include "PixelGraphicsAPI.h"
#include "RenderringData.h"
#include "BindManager.h"
#include <iostream>
extern PixelEngine* Engine;
BoxCollision2D::BoxCollision2D():collision(nullptr)
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
	collision->Push(SPointer<BoxCollision2D>::Make_SPointer());
}

void BoxCollision2D::Start()
{
	auto k = targetObject->GetModule<Transform>();
	transform = k.GetPtr();
}

void BoxCollision2D::PhysicsUpdate()
{
	auto t = transform;
	auto pos = t->GetPosition();

	rect.width = originalRect.width;
	rect.height = originalRect.height;

	// Transform 위치 + 사용자가 설정한 Offset - (크기의 절반)
	rect.x = (pos.X + originalRect.x) - (originalRect.width / 2.0f);
	rect.y = (pos.Y + originalRect.y) - (originalRect.height / 2.0f);
}

void BoxCollision2D::SetRect(float offsetX, float offsetY, float sizeX, float sizeY)
{
	originalRect.x = offsetX;
	originalRect.y = offsetY;
	originalRect.width = sizeX;
	originalRect.height = sizeY;

	rendering->collision.offsetX = offsetX;
	rendering->collision.offsetY = offsetY;
	rendering->collision.sizeX = sizeX;
	rendering->collision.sizeY = sizeY;
}

Rect BoxCollision2D::GetRect()
{
	return rect;
}

std::string BoxCollision2D::RegisterLua()
{
	auto state = GetLuaState();
	state->new_usertype<BoxCollision2D>("BoxCollision2D", sol::base_classes, sol::bases<Collision2D,Module, PixelObject>(),
		"SetRect", [](BoxCollision2D& obj, float offsetX, float offsetY, float sizeX, float sizeY) {obj.SetRect(offsetX, offsetY, sizeX, sizeY); }
	);
	
	std::string main = "";
	main += BindManager::ExportLuaAPIHeader<BoxCollision2D>();
	main += BindManager::ExportLuaAPIFromFunc("SetRect", &BoxCollision2D::SetRect, "offsetX,offsetY,sizeX,sizeY");
	return main;

}

void BoxCollision2D::LastUpdate()
{
	auto t = transform;

	// 2. 렌더링용 행렬 계산
	// 팁: 상용 엔진처럼 Offset(position)을 먼저 적용하고 부모의 World를 곱합니다.
	Matrix m = t->GetWorldMatrix();
	Matrix scale;
	Matrix position;

	// Scale(크기) -> Translation(오프셋) -> 부모의 WorldMatrix
	Matrix S = scale.CreateScale(Vector3( originalRect.width, originalRect.height, 1.0f));
	Matrix T = position.CreateTranslation(Vector3(originalRect.x, originalRect.y, 0.0f));

	Matrix finalWorld = S * T * m;

	for (int i = 0; i < 16; i++) {
		rendering->World[i] = finalWorld._m[i];
	}
}

void BoxCollision2D::OnCollisionEnter2D(WPointer<GameObject> target)
{

}

void BoxCollision2D::OnCollision2D(WPointer<GameObject> target)
{
	targetObject->OnCollision2D(target);
}

void BoxCollision2D::OnCollisionExit2D(WPointer<GameObject> target)
{

}
