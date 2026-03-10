#pragma once
#include <unordered_map>
#include <string>
#include "Core/PixelObject.h"
#include "Type/GlobalEnum.h"
#include "PixelEngineDLL.h"
#include <iostream>
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

	void test(int a, int b, int c) 
	{
		std::cout << "Test" << std::endl;
	}
	int test1()
	{
		std::cout << "Test1" << std::endl;
		return 0;
	}
public:
	std::string Save();
public:
	std::string name;
private:
	unsigned long hashCode;
	std::unordered_map<MODULE_TYPE,SPointer<Module>> ModuleMap;
	static FunctionManager* functionManager;
	friend FunctionManager;
};

