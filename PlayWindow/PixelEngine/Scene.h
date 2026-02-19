#pragma once
#include "PixelObject.h"
struct RenderingData;
class FunctionManager;
class Scene : public PixelObject
{
public:
	Scene();
	~Scene();
	void Initialize();
	void Start();
	void Update();
	void Release();
private:
	FunctionManager* manager;

	RenderingData* quad;
	RenderingData* cam;
};

