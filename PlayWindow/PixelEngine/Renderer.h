#pragma once
#include "Module.h"
struct RenderingData;
class Transform;
class Renderer : public Module
{
public:
	Renderer();
	~Renderer();

	void Awake() override;
	void Start() override;
	void Update() override;
	void LastUpdate() override;
private:
	RenderingData* rendering;
	Transform* transform;
	int index;
};

