#pragma once
#include "GraphicsCore.h"
struct RenderingData;
class Render_Quad : public GraphicsCore
{
public:
	Render_Quad();
	virtual ~Render_Quad();

	void Binding(RenderingData* mData);
};

