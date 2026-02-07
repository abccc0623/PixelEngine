#pragma once
struct RenderingData;
class FunctionManager;
class Scene
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

