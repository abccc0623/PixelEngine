#include "pch.h"
#include "PixelMetaAPI.h"
#include "PClass.h"

std::unordered_map<std::string, PClass*> ReflectionClass = std::unordered_map<std::string, PClass*>();

void ReflectionClassRegister(PClass* targetClass)
{
	auto find = ReflectionClass.find(targetClass->GetName());
	if (find == ReflectionClass.end())
	{
		ReflectionClass.insert({ targetClass->GetName() ,targetClass });
	}
}

void PixelMetaRelease()
{
	for (auto& K : ReflectionClass)
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