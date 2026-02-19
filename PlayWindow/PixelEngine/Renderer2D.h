#pragma once
#include "Module.h"
#include <string>
class Transform;
struct RenderingData;
using ObjectID = size_t;
class Renderer2D :public Module
{
public:
	Renderer2D();
	~Renderer2D();

	void Start() override;
	void LastUpdate() override;

	void SetTexture(const std::string& name);
	static void RegisterLua();
private:
	Transform* transform;
	RenderingData* rendering;
	std::string textureName;
	ObjectID textureID = -1;

};

