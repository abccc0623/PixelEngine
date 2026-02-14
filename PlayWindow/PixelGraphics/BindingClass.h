#pragma once
#include "KeyAllocator.h"
struct RenderingData;
class GraphicsEngine;
class BindingClass
{
public:
	BindingClass();
	virtual ~BindingClass();

	void SetEngine(GraphicsEngine* graphicsEngine);
	virtual void Initialize() = 0;
	virtual void Binding(RenderingData* mData, Handle64 prev) = 0;
	bool ModelCheck(Handle64 now, Handle64 prev);
	bool ShaderCheck(Handle64 now, Handle64 prev);
	bool TextureCheck(Handle64 now, Handle64 prev);
protected:
	GraphicsEngine* engine;
};

