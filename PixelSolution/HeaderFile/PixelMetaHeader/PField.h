#pragma once
#include <string>
#include <unordered_map>
#include "PValue.h"
#include "PixelMetaDLL.h"
#include <functional>

class PType;
using GetterFunc = std::function<PValue(void*,size_t)>;
class PField
{
public:
	PIXEL_META_DLL PField(PType* type, const std::string& name, size_t offset);
	PIXEL_META_DLL virtual ~PField();

	PIXEL_META_DLL std::string GetName();
	PIXEL_META_DLL std::string GetType();
	PIXEL_META_DLL PValue GetValuePtr(void* instance);
	PIXEL_META_DLL void SetValue(void* instance,PValue value);

	template<typename T>
	void SetValue(void* instance, const T& valuePtr)
	{
		SetValue(instance, const_cast<void*>(static_cast<const void*>(&valuePtr)));
	}
	PIXEL_META_DLL static void* operator new(size_t size);
	PIXEL_META_DLL static void operator delete(void* ptr);
	GetterFunc valueGetter;
private:
	PType* memberType;
	std::string memberName;
	size_t memberOffset;
};

