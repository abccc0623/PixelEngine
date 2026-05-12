#pragma once
namespace ECS::Collider2D
{
	enum Collider2DType
	{
		BOX,
		Circle,
	};
	struct Collider2DData
	{
		Collider2DType type = Collider2DType::BOX;
		Pixel::Vector2 Center = { 0.0f,0.0f };
		Pixel::Vector2 BoxOffset = { 0.5f,0.5f };
		float CircleRadius = 1.0f;
		bool IsChange = false;
		bool IsCreate = false;
	};
	void Add(unsigned int id);
	void SetBoxOffset(unsigned int id, float x, float y);
	void SetCircleOffset(unsigned int id, float radius);
	void SetCenter(unsigned int id, float x, float y);
}
