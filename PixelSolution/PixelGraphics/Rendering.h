#pragma once
#include "RenderringData.h"
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

	void SetRendering(RenderingData& data);
private:
	GraphicsEngine* graphicsEngine;

	std::vector<BindingClass*> bindingClassList;
	std::vector<RenderingData> renderingList;

	RenderingData Camera;
	Handle64 prevValue;
};

