#pragma once
#include "PixelMath.h"

namespace ECS::Transform
{
	struct TransformData
	{
		glm::vec4 position{ 0.0f, 0.0f, 0.0f, 1.0f };
		glm::vec4 rotation{ 0.0f, 0.0f, 0.0f, 0.0f };
		glm::vec4 scale{ 1.0f, 1.0f, 1.0f, 0.0f };
	};
	void Add(unsigned int id);
	void SetPosition(unsigned int id, float x, float y, float z);
	void SetRotation(unsigned int id, float x, float y, float z);
	void SetScale(unsigned int id, float x, float y, float z);
}