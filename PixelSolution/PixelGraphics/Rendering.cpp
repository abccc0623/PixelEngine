#include "pch.h"
#include "Rendering.h"

//Binding 클래스들

#include "RenderringData.h"
#include "ModelResources.h"
#include "GraphicsEngine.h"

#include "BindingClass.h"
#include "BindingCamera.h"
#include "BindingQuad.h"
#include "BindingRect.h"

#include <algorithm>

Rendering::Rendering()
{
	renderingList = std::vector<RenderingData>();
	renderingList.reserve(1000);
}

Rendering::~Rendering()
{
	
}

void Rendering::Initialize(GraphicsEngine* engine)
{
	graphicsEngine = engine;
	bindingClassList.resize(10);
	bindingClassList[RENDER_TYPE::CAMERA] = new BindingCamera();
	bindingClassList[RENDER_TYPE::QUAD] = new BindingQuad();
	bindingClassList[RENDER_TYPE::BOX2D] = new BindingRect();


	for (int i = 0; i < bindingClassList.size(); i++)
	{
		if (bindingClassList[i] == nullptr) { continue; }
		bindingClassList[i]->SetEngine(engine);
		bindingClassList[i]->Initialize();
	}
}

void Rendering::Release()
{
	for (auto K : bindingClassList)
	{
		if (K != nullptr)
		{
			delete K;
			K = nullptr;
		}
	}
	bindingClassList.clear();
}

void Rendering::Update()
{
	std::sort(renderingList.begin(), renderingList.end(), [](const RenderingData& a, const RenderingData& b)
		{
			return a.master_key < b.master_key;
		});

	bindingClassList[CAMERA]->Binding(&Camera, prevValue);

	int MaxSize = renderingList.size();
	for (int i = 0; i < MaxSize; i++)
	{
		switch (renderingList[i].Type)
		{
		case CAMERA:
			break;
		case QUAD:
			bindingClassList[QUAD]->Binding(&renderingList[i], prevValue);
			break;
		}
		prevValue = renderingList[i].master_key;
	}
	renderingList.clear();
	prevValue = 0;
}


void Rendering::SetRendering(RenderingData& data)
{
	if (data.Type == CAMERA)
	{
		Camera = data;
	}
	else
	{
		data.Setting();
		renderingList.push_back(data);
	}
}


