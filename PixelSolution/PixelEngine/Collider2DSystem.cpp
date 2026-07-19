#include "pch.h"
#include "Collider2DSystem.h"
#include "PixelGraphicsAPI.h"
#include "Registry.h"
#include "PhysManager.h"
#include "PixelEngine.h"
extern PixelEngine* Engine;
ECS::Collider2DSystem::Collider2DSystem()
{
	phys = Engine->GetFactory<PhysManager>();
}

ECS::Collider2DSystem::~Collider2DSystem()
{

}

void ECS::Collider2DSystem::Update(Registry* registry)
{

}

void ECS::Collider2DSystem::EditorUpdate(Registry* registry)
{


}

void ECS::Collider2DSystem::Release()
{

}
