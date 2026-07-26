#pragma once
#include <cstdint>
#include <windows.h>

#include "GraphicsCore.h"
#include "ResourceManager.h"
#include "RenderingManager.h"

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

class RenderingManager;
class Factory;
class ResourceFactory;
class DrawDebugLine;
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
	void Clear();
public:
	PixelGraphics::ResourceManager* GetResourceManager();
	PixelGraphics::RenderingManager* GetRenderingManager();
	PixelGraphics::GraphicsCore* GetGraphicsCore();


	//void SetRenderingData(RenderingData& mData);
	//void DrawLine(float start[3], float end[3], float color[3]);
	//GraphicsCore& GetCore() { return graphicsCore; }
	//const GraphicsCore& GetCore() const { return graphicsCore; }
	//
	//ObjectID LoadTexture(const char* filePath);
	//ObjectID LoadMaterial(const char* filePath);
	//ObjectID Model_Debug(float* VertexList, int VertexSize, int* IndexList, int indexSize);
	//ObjectID Model_Debug(Vertex_Debug* VertexList, int VertexSize, int* IndexList, int indexSize);


	//std::unordered_map<std::type_index, ResourceFactory*> factoryMap;
	//
	//template<typename T1, std::derived_from<ResourceFactory> T2>
	//void BindFactory()
	//{
	//	std::type_index key = typeid(T1);
	//	auto k = factoryMap.find(key);
	//	if (k == factoryMap.end())
	//	{
	//		auto factory = new T2();
	//		factoryMap.insert({ key ,factory });
	//	}
	//}
	//
	//template<typename T>
	//T* Get(const std::string& name)
	//{
	//	std::type_index key = typeid(T);
	//	auto k = factoryMap.find(key);
	//	if (k != factoryMap.end())
	//	{
	//		auto* data = factoryMap[key]->GetResource(name);
	//		return static_cast<T*>(data);
	//	}
	//	return nullptr;
	//}
	//template<typename T>
	//T* Get(std::uint16_t handleKey)
	//{
	//	std::type_index key = typeid(T);
	//	auto k = factoryMap.find(key);
	//	if (k != factoryMap.end())
	//	{
	//		auto* data = factoryMap[key]->GetResource(handleKey);
	//		return static_cast<T*>(data);
	//	}
	//	return nullptr;
	//}
	//
	//template<typename T>
	//std::uint16_t Set(const std::string& name)
	//{
	//	std::type_index key = typeid(T);
	//	auto k = factoryMap.find(key);
	//	if (k != factoryMap.end())
	//	{
	//		return factoryMap[key]->SetResource(name);
	//	}
	//	return 0;
	//}
	//
	//template<typename T1, std::derived_from<ResourceFactory> T2>
	//T2* GetFactory()
	//{
	//	std::type_index key = typeid(T1);
	//	auto k = factoryMap.find(key);
	//	if (k != factoryMap.end())
	//	{
	//		auto* data = factoryMap[key];
	//		return static_cast<T2*>(data);
	//	}
	//	return nullptr;
	//}
private:
	PixelGraphics::GraphicsCore graphicsCore;
	PixelGraphics::ResourceManager resourceManager;
	PixelGraphics::RenderingManager renderingManager;
	DrawDebugLine* mDebugRender;
};
