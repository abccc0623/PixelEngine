#pragma once
#include <string>
#include <unordered_map>
#include <functional>
#include "PixelMetaAPI.h"
#include "PixelMetaDLL.h"
#include "PValue.h"

template <typename T>
struct FunctionTraits;

// 멤버 함수용 특수화
template <typename Ret, typename Class, typename... Args>
struct FunctionTraits<Ret(Class::*)(Args...)>
{
	using ReturnType = Ret;
	using ArgsTuple = std::tuple<Args...>;
};


using MethodPtr = std::function<PValue (void*,std::vector<void*>&)>;
class PField;
class PMethod
{
public:
	PIXEL_META_DLL PMethod(std::string name, PType* returnType, std::vector<PType*> memberList);
	PIXEL_META_DLL ~PMethod();


	PIXEL_META_DLL PValue Invoke(void* instance, std::vector<void*> members);
	PIXEL_META_DLL static void* operator new(size_t size);
	PIXEL_META_DLL static void operator delete(void* ptr);
	MethodPtr templateFunction;


	template<typename ReturnType,typename TClass, typename F, size_t... Is>
	static PValue ReturnCallHelper(TClass* obj, F func, std::vector<void*>& args, std::index_sequence<Is...>) 
	{
		//if constexpr (sizeof...(TArgs) == 0)
		//{
		//	//매개변수 X
		//	ReturnType res = (obj->*func)();
		//	return PValue(res);
		//}
		//else
		//{
		//	//매개변수 O
		//	ReturnType res = (obj->*func)((*static_cast<std::decay_t<TArgs>*>(args[Is]))...);
		//	return PValue(res);
		//}
	}



	template<typename TClass, typename F, size_t... Is>
	static PValue CallHelper(TClass* obj,F func,std::vector<void*>& args, std::index_sequence<Is...>)
	{
		//std::declval<TArgs>()...) 인자가 해당 함수로 들어왔다고 가정..
		//decltype((obj->*func)(std::declval<TArgs>()...) //가상함수 맴버 함수 객체를 통해 실행하는 틀을제작
		//decltype 함수의 모양을 보고 실제 return 타입을 추론
		//using ReturnType = decltype((obj->*func)(std::declval<TArgs>()...));
		//
		//if constexpr (std::is_void_v<ReturnType>)
		//{
		//	//반환형이 void
		//	if constexpr (sizeof...(TArgs) == 0)
		//	{
		//		//매개변수 X
		//		(obj->*func)();
		//	}
		//	else
		//	{
		//		//매개변수 O
		//		(obj->*func)((*static_cast<std::decay_t<TArgs>*>(args[Is]))...);
		//	}
		//	return PValue();
		//}
		//else
		//{
		//	//반환형 존재 ReturnType
		//	ReturnType res;
		//	if constexpr (sizeof...(TArgs) == 0)
		//	{
		//		//매개변수 X
		//		ReturnType res = (obj->*func)();
		//		return PValue(res);
		//	}
		//	else
		//	{
		//		//매개변수 O
		//		ReturnType res = (obj->*func)((*static_cast<std::decay_t<TArgs>*>(args[Is]))...);
		//		return PValue(res);
		//	}
		//	return PValue();
		//}
		return PValue();
	}

protected:
	std::string methodName;
	std::vector<PType*> memberList;
	PType* memberReturn;
};


