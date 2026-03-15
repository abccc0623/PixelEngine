#pragma once
#include <unordered_map>
#include <string>
#include "Core/PixelObject.h"
#include "PixelEngineDLL.h"

template <typename T> class SPointer;
class Module;
class FunctionManager;
class PClass;
class GameObject :public PixelObject
{
public:
	GameObject();
	virtual ~GameObject();
public:
	PIXEL_ENGINEDLL Module* AddModule(PClass* moduleClass);
	PIXEL_ENGINEDLL bool HasModule(PClass* moduleClass);
	PIXEL_ENGINEDLL Module* GetModule(PClass* moduleClass);
	PIXEL_ENGINEDLL unsigned long GetHash();
	std::string name;
private:
	unsigned long hashCode;
	std::unordered_map<uint64_t,SPointer<Module>> ModuleMap;
private:
	static FunctionManager* functionManager;
	friend FunctionManager;
};

