#pragma once
#include "Module.h"
class Transform;
class RenderingData;
class Renderer2D :public Module
{
public:
	Renderer2D();
	~Renderer2D();

	void Awake() override;
	void Start() override;
	void Update() override;
	void LastUpdate() override;
private:
	Transform* transform;
	RenderingData* rendering;
};

