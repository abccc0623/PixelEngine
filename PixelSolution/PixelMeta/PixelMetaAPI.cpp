#include "pch.h"
#include "PixelMetaAPI.h"
#include "PType.h"
#include "PClass.h"
#include "PEnum.h"
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


int PixelMeta_GetFieldMaxCount(PObject* target)
{
	return target->GetFieldMaxCount();
}

int PixelMeta_GetMethodMaxCount(PObject* target)
{
	return target->GetMethodMaxCount();
}

const char* PixelMeta_TypeName(PObject* target)
{
	const std::string& name = target->TypeName();
	return name.c_str();
}

const char* PixelMeta_ParentName(PObject* target)
{
	return target->ParentName().c_str();
}

const char* PixelMeta_ChildName(PObject* target)
{
	return target->ChildName().c_str();
}

const char* PixelMeta_GetFieldName(PObject* target, int index)
{
	return target->GetFieldName(index).c_str();
}

const char* PixelMeta_GetFieldType(PObject* target, int index)
{
	return target->GetFieldType(index).c_str();
}


