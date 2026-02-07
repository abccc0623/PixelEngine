#pragma once
#include "GraphicsCore.h"
struct RenderingData;
class Render_Camera : public GraphicsCore
{
public:
	Render_Camera();
	virtual ~Render_Camera();

	void Binding(float* m);
	void Binding(RenderingData* mData);
private:
	float Position[3];
};

