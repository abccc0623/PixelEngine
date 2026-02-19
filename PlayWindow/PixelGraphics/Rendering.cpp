#include "Rendering.h"

//Binding 클래스들

#include "RenderringData.h"
#include "ModelResources.h"
#include "GraphicsEngine.h"

#include "BindingClass.h"
#include "BindingCamera.h"
#include "BindingQuad.h"
#include "BindingRect.h"

#include <limits>
#include <string>
#include <bitset>
#include <iostream>
#include <algorithm>
Rendering::Rendering()
{
	renderingList = std::map<Handle64, std::vector<RenderingData*>>();
	bindingClassList = std::vector<BindingClass*>();
}

Rendering::~Rendering()
{
	for (auto& K : renderingList)
	{
		for (auto& Q : K.second)
		{
			delete Q;
			Q = nullptr;
		}
	}
	renderingList.clear();
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
	for (auto K : renderingList)
	{
		for (int i = 0; i < K.second.size(); i++)
		{
			auto value = K.second[i];
			delete value;
			K.second[i] = nullptr;
		}
		K.second.clear();
	}
	renderingList.clear();
}

void Rendering::Update()
{
	for (auto& K : renderingList)
	{
		if (K.first == maxHandle64)
		{
			for (int i = (int)K.second.size() - 1; i >= 0; i--)
			{
				auto& value = K.second[i];
				SettingData(value);
				K.second.erase(K.second.begin() + i);
				ChangePushBack(value);
			}
			K.second.clear();
		}
	}

	prevValue = maxHandle64;
	for (auto& K : renderingList) 
	{
		for (int i = 0; i < K.second.size(); i++)  
		{
			switch (K.second[i]->Type)
			{
			case QUAD:
				bindingClassList[QUAD]->Binding(K.second[i], prevValue);
				break;
			case CAMERA:
				bindingClassList[CAMERA]->Binding(K.second[i], prevValue);
				break;
			case BOX2D:
				bindingClassList[BOX2D]->Binding(K.second[i], prevValue);
				break;
			}
			prevValue = K.second[i]->master_key;
		}
	}
}
void Rendering::SettingData(RenderingData* data)
{
	data->master_key = 0;
	switch (data->Type)
	{
	case QUAD:
	{
		auto model = graphicsEngine->Get<DirectModel>("Quad");
		auto shader = graphicsEngine->Get<ShaderResources>("Static");
		data->mesh.model_key = model->key;
		data->mesh.shader_key = shader->key;
		break;
	}
	case BOX2D:
	{
		auto model = graphicsEngine->Get<DirectModel>("Box2D");
		auto shader = graphicsEngine->Get<ShaderResources>("Debug");
		data->mesh.model_key = model->key;
		data->mesh.shader_key = shader->key;
		break;
	}
	default:
	{
		return;
	}
	}
	Handle64 model = (Handle64)data->mesh.model_key;
	Handle64 shader = (Handle64)data->mesh.shader_key;
	Handle64 texture = (Handle64)data->mesh.texture_key;
	model <<= 0;
	shader <<= 16;
	texture <<= 32;
	data->master_key |= model;
	data->master_key |= shader;
	data->master_key |= texture;
	std::cout << "Binary Log: " << std::bitset<64>(data->master_key) << std::endl;
}

void Rendering::SetRendering(RenderingData* data)
{
	auto find = renderingList.find(data->master_key);
	if (find == renderingList.end())
	{
		data->master_key = maxHandle64;
		renderingList.insert({ data->master_key,std::vector<RenderingData*>()});
		renderingList[data->master_key].push_back(data);
	}
	else 
	{
		renderingList[data->master_key].push_back(data);
	}
}

void Rendering::DeleteRendering(RenderingData* data)
{
	auto find = renderingList.find(data->master_key);
	if (find != renderingList.end())
	{
		std::erase(renderingList[data->master_key], data);
	}
}

void Rendering::ChangePushBack(RenderingData* data) 
{
	auto find = renderingList.find(data->master_key);
	if (find == renderingList.end())
	{
		renderingList.insert({ data->master_key,std::vector<RenderingData*>() });
		renderingList[data->master_key].push_back(data);
	}
	else
	{
		renderingList[data->master_key].push_back(data);
	}
}

