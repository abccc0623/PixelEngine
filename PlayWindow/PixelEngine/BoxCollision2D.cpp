#include "BoxCollision2D.h"
#include "PixelEngine.h"
#include "CollisionManager.h"
#include <iostream>
extern PixelEngine* Engine;
BoxCollision2D::BoxCollision2D():
	collision(nullptr)
{

}

BoxCollision2D::~BoxCollision2D()
{

}

void BoxCollision2D::Awake()
{
	collision = Engine->GetFactory<CollisionManager>();
}

void BoxCollision2D::Start()
{
	std::cout << "BoxCollision2D : Start" << std::endl;
}

void BoxCollision2D::PhysicsUpdate()
{

}

void BoxCollision2D::RegisterLua()
{

}
