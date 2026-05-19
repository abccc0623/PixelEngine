#pragma once
namespace ECS::CircleCollider2D
{
	struct CircleCollider2DData
	{
		Pixel::Vector2 Center = { 0.0f,0.0f };
		float Radius = 0.5;
	};
	void* AddComponent(unsigned int id);
	void* GetComponent(unsigned int id);
	bool HasComponent(unsigned int id);
	std::string BindJit();
}
