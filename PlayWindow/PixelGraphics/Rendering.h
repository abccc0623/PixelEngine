#pragma once

#include "GraphicsCore.h"
#include "KeyAllocator.h"
#include <map>
#include <vector>
class BindingCamera;
class Render_Axis;
class Render_Quad;
struct RenderingData;
class GraphicsEngine;
class BindingClass;
class Rendering
{
public:
	Rendering();
	virtual ~Rendering();

	void Initialize(GraphicsEngine* engine);
	void Release();
	void Update();

	void SettingData(RenderingData* data);
	void SetRendering(RenderingData* data);
	void DeleteRendering(RenderingData* data);
private:
	void ChangePushBack(RenderingData* data);
	const Handle64 maxHandle64 = ~0ULL;
	GraphicsEngine* graphicsEngine;
	std::map<Handle64,std::vector<RenderingData*>> renderingList;
	std::vector<BindingClass*> bindingClassList;
	Handle64 prevValue;
};

