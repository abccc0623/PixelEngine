#pragma once

#include "BindingClass.h"
class GraphicsEngine;
struct RenderingData;
struct BufferResources;
class BindingCamera : public BindingClass
{
public:
	BindingCamera();
	virtual ~BindingCamera();

	void Initialize() override;
	//void Binding(float* m);
	void Binding(RenderingData* mData, Handle64 prev) override;
private:
	BufferResources* cameraBuffer;
};

