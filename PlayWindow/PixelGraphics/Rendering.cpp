#include "Rendering.h"

//Binding 클래스들
#include "Render_Camera.h"
#include "Render_Axis.h"
#include "Render_Quad.h"
#include "RenderringData.h"
Rendering::Rendering() :
	mRenderCamera(nullptr),
	mRenderAxis(nullptr),
	mRenderQuad(nullptr)
{

}

Rendering::~Rendering()
{

}

void Rendering::Initialize()
{
	mRenderCamera	= new Render_Camera();
	mRenderAxis		= new Render_Axis();
	mRenderQuad		= new Render_Quad();
}

void Rendering::Release()
{
	delete mRenderCamera;
	delete mRenderAxis;
	delete mRenderQuad;
}

void Rendering::Update()
{
	for(auto& K : mRendering_List)
	{
		if (K == nullptr)continue;

		switch (K->Type)
		{
		case RENDER_TYPE::NONE:
			break;
		case RENDER_TYPE::DEBUG:
			mRenderAxis->Binding(K);
			break;
		case RENDER_TYPE::CAMERA:
			mRenderCamera->Binding(K);
			break;
		case RENDER_TYPE::QUAD:
			mRenderQuad->Binding(K);
			break;
		default:
			break;
		}

	}
}
