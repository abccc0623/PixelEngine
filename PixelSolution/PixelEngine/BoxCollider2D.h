#pragma once

namespace ECS::BoxCollider2D
{
	struct BoxCollider2DData
	{
		Pixel::Vector2 Center = { 0.0f,0.0f };
		Pixel::Vector2 Offset = { 0.5f,0.5f };
	};
	void* AddComponent(unsigned int id);
	void* GetComponent(unsigned int id);
	bool HasComponent(unsigned int id);
	std::string BindJit();
}