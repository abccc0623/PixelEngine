#include "ModelFactory.h"
#include <iostream>
void ModelFactory::Initialize()
{
	defaultModelMap.insert({ "Quad",CreateQuad() });
}

void ModelFactory::Release()
{
	for (auto& k : ModelMap)
	{
		k.second->IndexBuffer->Release();
		k.second->VertexBuffer->Release();
		delete k.second;
		k.second = nullptr;
	}
	for (auto& k : defaultModelMap)
	{
		k.second->IndexBuffer->Release();
		k.second->VertexBuffer->Release();
		delete k.second;
		k.second = nullptr;
	}
	ModelMap.clear();
	defaultModelMap.clear();
}

void* ModelFactory::GetResource(std::string name)
{
	auto k = defaultModelMap.find(name.c_str());
	if (k == defaultModelMap.end())
	{
		std::cout << "Not Find Shader :" + name << std::endl;
		return nullptr;
	}
	else
	{
		return k->second;
	}
}


DirectModel* ModelFactory::CreateQuad()
{
	StaticVertex Debug[4];
	int Index[6] = { 0, 1, 2, 0, 2, 3 };

	Debug[0].Pos = { -0.5f,0.5f,0 };
	Debug[0].UV = { 0.0f,0.0f };

	Debug[1].Pos = { 0.5f,0.5f,0 };
	Debug[1].UV = { 1.0f,0.0f };

	Debug[2].Pos = { 0.5f,-0.5f,0 };
	Debug[2].UV = { 1.0f, 1.0f };

	Debug[3].Pos = { -0.5f,-0.5f,0 };
	Debug[3].UV = { 0.0f, 1.0f };
	auto model = CreateModelBuffer(Debug, 4, Index, 6);
	model->key = keyAllocator.GetKey16();
	return model;
}