#pragma once

#include "GraphicsCore.h"
#include <map>
class Render_Camera;
class Render_Axis;
class Render_Quad;
class Rendering : public GraphicsCore
{
public:
	Rendering();
	virtual ~Rendering();

	void Initialize();
	void Release();
	void Update();

	float* DebugCameraMatrix;
	float* MainCameraMatrix;
private:
	Render_Camera* mRenderCamera;
	Render_Axis* mRenderAxis;
	Render_Quad* mRenderQuad;
};

