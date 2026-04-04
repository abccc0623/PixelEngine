#include "pch.h"
#include "Movement.h"
#include "PixelEngine.h"
#include "SceneManager.h"
#include "Module/Transform.h"
#include "Core/GameObject.h"
#include "Type/PVector3.h"
#include "Scene.h"
extern PixelEngine* Engine;
Movement::Movement()
{

}

Movement::~Movement()
{

}

void Movement::Update()
{
	if (target == nullptr)return;
	if (target.IsValid())
	{
		auto k  = target.Lock();
		auto tr = k->GetTransform();
		
		auto distance =  Distance(transform->Position, tr->Position);
		auto direction = tr->Position - transform->Position;

		if (distance > StopDistance)
		{
			transform->Position += direction * (GetDeltaTime() * speed);
		}
	}
	else
	{
			
	}
}

void Movement::MoveToTarget(GameObject* target)
{
	auto manager = Engine->GetFactory<SceneManager>();
	auto scene =  manager->GetNowScene();
	this->target = scene->FindGameObjectToEngine(target);
}
