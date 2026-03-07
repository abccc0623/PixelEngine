#include "pch.h"
#include "PixelMetaAPI.h"
#include "PType.h"
#include "RSystem.h"

RSystem* System = nullptr;
template<typename T>
void StartRegisterType()
{
	std::string name = ExtractTypeName<T>();
	uint64_t hash = HashUtil::ConstexprHash(name.c_str());
	size_t size = sizeof(T);
	RSystem::MapByHash.insert({ hash,new PType(hash, name, size) });
}

void PixelMetaRelease()
{
	if (System == nullptr)
	{
		return;
	}
	System->Release();
}

void RegisterClassType(PType* type)
{
	if (System == nullptr)
	{
		System = new RSystem();
	}
	System->Register(type);
}

PType* GetTypeByString(std::string name)
{
	uint64_t hash = HashUtil::ConstexprHash(name.c_str());
	if (System == nullptr)
	{
		System = new RSystem();
	}
	return System->GetType(hash);
}

void GenerateClassFile(Generatelanguage type, std::string Path)
{
	if (System == nullptr)
	{
		System = new RSystem();
	}
}

PObject* CreateRObject(void* target, std::string TypeName)
{
	if (System == nullptr)
	{
		System = new RSystem();
	}
	return System->CreateObject(target, TypeName);
}
