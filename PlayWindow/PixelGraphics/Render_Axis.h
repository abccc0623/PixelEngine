#pragma once
#include "GraphicsCore.h"
struct RenderingData;
class Render_Axis : public GraphicsCore
{
public:
	Render_Axis();
	virtual ~Render_Axis();

	void Binding(RenderingData* mData);
};

