#pragma once
#include <string>
#include <unordered_map>
#include <functional>
#include "PixelMetaDLL.h"
#include "PValue.h"

class PField;
class PMethod
{
public:
	PIXEL_META_DLL PMethod(std::string name);
	PIXEL_META_DLL ~PMethod();


	PIXEL_META_DLL PValue Invoke(void* instance, std::vector<void*>& members);
	PIXEL_META_DLL static void* operator new(size_t size);
	PIXEL_META_DLL static void operator delete(void* ptr);


	template<typename TClass, typename F, typename... Args, size_t... Is>
	static void CallHelper(TClass* obj,F func,std::vector<void*>& args, std::index_sequence<Is...>)
	{
		if (args.size() == 0)
		{
			(obj->*func)();
		}
		else
		{
			(obj->*func)((*static_cast<std::remove_reference_t<Args>*>(args[Is]))...);
		}
	}

	template<typename TClass, typename F, typename... Args, size_t... Is>
	static PValue ReturnCallHelper(TClass* obj, F func, std::vector<void*>& args, std::index_sequence<Is...>)
	{
		if (args.size() == 0)
		{
			return PValue((obj->*func)());
		}
		else
		{
			return PValue((obj->*func)((*static_cast<std::remove_reference_t<Args>*>(args[Is]))...));
		}
	}

	std::function<void*(void*, std::vector<void*>&)> NoReturnFunction;
protected:
	std::string methodName;
};


