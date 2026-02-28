#pragma once
#include <unordered_map>
#include <string>
#include "Core/PixelObject.h"
#include "Type/GlobalEnum.h"
#include "PixelEngineDLL.h"
template <typename T> class SPointer;
class Module;
class BindManager;
class ObjectManager;
class FunctionManager;

class GameObject :public PixelObject
{
public:
	GameObject();
	virtual ~GameObject();
public:
	PIXEL_ENGINEDLL void Destroy();
	PIXEL_ENGINEDLL void AddModule(MODULE_TYPE type); 
	PIXEL_ENGINEDLL bool HasModule(MODULE_TYPE type);
	PIXEL_ENGINEDLL Module* GetModule(MODULE_TYPE type);
	PIXEL_ENGINEDLL unsigned long GetHash();
public:
#ifdef PIXEL_EDITOR
	PIXEL_ENGINEDLL std::string Save(int tab);
#endif
public:
	std::string name;
private:
	unsigned long hashCode;
	std::unordered_map<MODULE_TYPE,SPointer<Module>> ModuleMap;
	static FunctionManager* functionManager;
	friend FunctionManager;
};

