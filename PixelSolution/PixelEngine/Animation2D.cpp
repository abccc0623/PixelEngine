#include "pch.h"
#include "Animation2D.h"
#include "Registry.h"
#include "PixelEngineAPI.h"
void ECS::Animation2D::Add(unsigned int id)
{
	auto registry = GetRegistry();
	registry->AddComponent<Animation2DData>(id);
}

void ECS::Animation2D::CreateAnimation(unsigned int id, std::string textureName, int MaxFramesX, int MaxFramesY, float speed)
{


}

void ECS::Animation2D::PlayAnimation(unsigned int id, int index)
{


}
