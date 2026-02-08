#pragma once
#include "Module.h"
#include "BindLua.h"
#include <string>
class Transform;
class RenderingData;
using ObjectID = size_t;
class Renderer2D :public Module ,public BindLua<Renderer2D>
{
public:
	Renderer2D();
	~Renderer2D();

	void Awake() override;
	void Start() override;
	void Update() override;
	void LastUpdate() override;

	void SetTexture(const std::string& name);
	static void RegisterLua();
private:
	Transform* transform;
	RenderingData* rendering;
	std::string textureName;
	ObjectID textureID = -1;

};

