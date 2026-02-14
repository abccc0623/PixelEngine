#pragma once
#include "KeyAllocator.h"
#include <type_traits>
#include <unordered_map>
#include <string>

class ResourceFactory
{
public:
	ResourceFactory() = default;
	virtual ~ResourceFactory() = default;

	virtual void Initialize()	= 0;
	virtual void Release()		= 0;

	virtual void* GetResource(std::string name) = 0;
	virtual void* GetResource(Handle16 key);

	virtual Handle16 SetResource(std::string name);
protected:
	template <typename T>
	size_t CreateHashCode(T* taget)
	{
		return std::hash<T*>{}(taget);
	}
	KeyAllocator keyAllocator;
};


