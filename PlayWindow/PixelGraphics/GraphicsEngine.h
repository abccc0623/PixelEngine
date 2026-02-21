#pragma once
#include <windows.h>
#include "ResourceFactory.h"
#include "PixelResources.h"
#include <unordered_map>
#include <typeindex>
using ObjectID = size_t;

class RasterizerStateFactory;
class ShaderFactory;
class TextureFactory;
class ModelFactory;
class RenderingFactory;

class Rendering;
class Factory;
class ResourceFactory;
struct RenderingData;
struct Vertex_Debug;
struct DirectModel;

#include <vector>
class GraphicsEngine
{
public:
	GraphicsEngine();
	~GraphicsEngine();
public:
	void Initialize(HWND WindowHandle, int Width, int Height);
	void Release();
	void BeginRender(float R, float G, float B, float A);
	void EndRender();
	
	RenderingData* GetRenderingData();
	void DeleteRenderingData(RenderingData* mData);
	void ChangeRenderingData(RenderingData* mData);


	ObjectID LoadTexture(const char* filePath);
	ObjectID Model_Debug(float* VertexList, int VertexSize, int* IndexList, int indexSize);
	ObjectID Model_Debug(Vertex_Debug* VertexList, int VertexSize, int* IndexList, int indexSize);


	std::unordered_map<std::type_index, ResourceFactory*> factoryMap;

	template<typename T1, std::derived_from<ResourceFactory> T2>
	void BindFactory()
	{
		std::type_index key = typeid(T1);
		auto k = factoryMap.find(key);
		if (k == factoryMap.end())
		{
			auto factory = new T2();
			factoryMap.insert({ key ,factory });
		}
	}
	
	template<typename T>
	T* Get(const std::string& name)
	{
		std::type_index key = typeid(T);
		auto k = factoryMap.find(key);
		if (k != factoryMap.end())
		{
			auto* data = factoryMap[key]->GetResource(name);
			return static_cast<T*>(data);
		}
		return nullptr;
	}
	template<typename T>
	T* Get(Handle16 handleKey)
	{
		std::type_index key = typeid(T);
		auto k = factoryMap.find(key);
		if (k != factoryMap.end())
		{
			auto* data = factoryMap[key]->GetResource(handleKey);
			return static_cast<T*>(data);
		}
		return nullptr;
	}

	template<typename T>
	Handle16 Set(const std::string& name)
	{
		std::type_index key = typeid(T);
		auto k = factoryMap.find(key);
		if (k != factoryMap.end())
		{
			return factoryMap[key]->SetResource(name);
		}
		return 0;
	}

	template<typename T1, std::derived_from<ResourceFactory> T2>
	T2* GetFactory() 
	{
		std::type_index key = typeid(T1);
		auto k = factoryMap.find(key);
		if (k != factoryMap.end()) 
		{
			auto* data = factoryMap[key];
			return static_cast<T2*>(data);
		}
		return nullptr;
	}
private:
	Rendering* mRender;
};

