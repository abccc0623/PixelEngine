#include "pch.h"
#include "GlobalMetaFunc.h"
#include "PixelMetaAPI.h"
#include "PFieldData.h"
#include "PClass.h"

std::unordered_map<uint64_t, PClass*> ReflectionClassByKeyword = std::unordered_map<uint64_t, PClass*>();
std::unordered_map<uint64_t, PClass*> ReflectionClassByString = std::unordered_map<uint64_t, PClass*>();

void ReflectionClassRegister(uint64_t hash,PClass* targetClass)
{
	auto find = ReflectionClassByKeyword.find(hash);
	if (find == ReflectionClassByKeyword.end())
	{
		ReflectionClassByKeyword.insert({ hash ,targetClass });

		uint64_t nameHash = HashUtil::ConstexprHash(targetClass->GetName().c_str());
		ReflectionClassByString.insert({ nameHash,targetClass });
	}
}

PClass* FindClassByName(std::string ClassName)
{
	uint64_t nameHash = HashUtil::ConstexprHash(ClassName.c_str());
	auto find = ReflectionClassByString.find(nameHash);
	if (find != ReflectionClassByString.end())
	{
		return find->second;
	}
	else
	{
		return nullptr;
	}
}

PClass* FindClassByHash(uint64_t ClassHash)
{
	auto find = ReflectionClassByKeyword.find(ClassHash);
	if (find != ReflectionClassByKeyword.end())
	{
		return find->second;
	}
	else
	{
		return nullptr;
	}
}

void PixelMetaRelease()
{
	for (auto& K : ReflectionClassByKeyword)
	{
		K.second->Release();
		delete K.second;
	}
}

void* PixelInternalAlloc(size_t size)
{
	return ::operator new(size);
}

void PixelInternalFree(void* ptr)
{
	if (ptr) ::operator delete(ptr);
}

PField* CreateMetaType(uint64_t hash,const std::string& fieldName, size_t offset, size_t size)
{
	switch (hash)
	{
	case TypeHash<int>::Value():			return new PFieldData<int>("int", fieldName, offset);
	case TypeHash<float>::Value():			return new PFieldData<float>("float", fieldName, offset);
	case TypeHash<std::string>::Value():	return new PFieldData<std::string>("string", fieldName, offset);
	
	//모르는 타입에 대한 정의
	default:
		return new PFieldObject("class", fieldName, offset, size);
	}
	return nullptr;
}

