#pragma once
#include <string>
#include <vector>
#include <typeindex>
#include <typeinfo>
#include <functional>
#include "PixelMetaDLL.h"
#include "GlobalHashCode.h"
#include "PValue.h"

struct MemberInfo
{
	std::string memberType;
	size_t offset;
};

struct MethodInfo
{
	std::string retrunType;
	std::string classType;
	std::vector<std::string> memberType;
	std::function<PValue (void*, std::vector<void*>&)> invoker;
};

///
extern "C" PIXEL_META_DLL uint64_t StringToByHash(const char* name);

class PClass;
///클래스 필수 함수
extern "C" PIXEL_META_DLL PClass* CreateNewClass(const std::string& thidTypeName, const std::string& parentTypeName = "");
extern "C" PIXEL_META_DLL void CreateClassFunction(PClass* targetClass,void*(createFunc)());
extern "C" PIXEL_META_DLL void DeleteClassFunction(PClass* targetClass,void(deleteFunc)());
extern "C" PIXEL_META_DLL void CastClassFunction(PClass* targetClass, void*(castFunc)());
extern "C" PIXEL_META_DLL int AllClassCount();
extern "C" PIXEL_META_DLL PClass* GetClassByIndex(int index);
extern "C" PIXEL_META_DLL const std::string& GetClassTypeName(PClass* targetClass);

///클래스 타입생성
extern "C" PIXEL_META_DLL PClass* GetClass(const std::string& className);
extern "C" PIXEL_META_DLL uint64_t GetClassHashByString(const std::string& className);
extern "C" PIXEL_META_DLL uint64_t GetClassHash(PClass* targetClass);
extern "C" PIXEL_META_DLL uint64_t GetClassParentHash(PClass* targetClass);
extern "C" PIXEL_META_DLL uint64_t GetClassChildHash(PClass* targetClass);

///클래스 정보 등록
extern "C" PIXEL_META_DLL bool AddMember(PClass* targetClass, const std::string& memberName, MemberInfo info,long flag = 0);
extern "C" PIXEL_META_DLL bool AddMethod(PClass* targetClass,const std::string& methodName, MethodInfo info, long flag = 0);

///클래스 맴버 정보
extern "C" PIXEL_META_DLL const std::string& GetClassMemberName(PClass* targetClass,int index);
extern "C" PIXEL_META_DLL const std::string& GetClassMemberType(PClass* targetClass,int index);
extern "C" PIXEL_META_DLL bool  HasClassMemberFlag(PClass* targetClass,int index,long flag);
extern "C" PIXEL_META_DLL void* GetClassMemberValue(PClass* targetClass,int index,void* target);
extern "C" PIXEL_META_DLL void  SetClassMemberValue(PClass* targetClass,int index,void* target,void* value);
extern "C" PIXEL_META_DLL int GetClassMemberCount(PClass* targetClass);

///클래스 함수 정보
extern "C" PIXEL_META_DLL const std::string& GetClassMethodName(PClass* targetClass, int index);
extern "C" PIXEL_META_DLL const std::string& GetClassMethodReturnType(PClass* targetClass, int index);
extern "C" PIXEL_META_DLL const std::string& GetClassMethodGetPropertyType(PClass* targetClass, int index,int propertyIndex);
extern "C" PIXEL_META_DLL bool HasClassMethodFlag(PClass* targetClass, int index, long flag);
extern "C" PIXEL_META_DLL int GetClassMethodPropertyCount(PClass* targetClass, int index);
extern "C" PIXEL_META_DLL int GetClassMethodCount(PClass* targetClass);
extern "C" PIXEL_META_DLL PValue CallClassMethod(PClass* targetClass,int index,void* target,std::vector<void*> property);

///타입 정리
extern "C" PIXEL_META_DLL void ReleaseMetaType();


//클래스에서 맴버의 Offset을 구하는 편의 기능
template<typename TClass,typename FieldType>
MemberInfo GetMemberInfo(FieldType TClass::* memberPtr)
{
	MemberInfo info;
	info.offset = reinterpret_cast<size_t>(&(((TClass*)0)->*memberPtr));
	info.memberType = ExtractTypeName<FieldType>();
	return info;
}


template <typename T>
std::string GetCleanTypeName() 
{
	using Cleaned = std::remove_cv_t<std::remove_reference_t<T>>;
	return ExtractTypeName<Cleaned>();
}

template<typename ClassType, typename ReturnType, typename... Args>
MethodInfo GeMethodInfo(ReturnType(ClassType::*Func)(Args...))
{
	MethodInfo info;

	info.classType = ExtractTypeName<ClassType>();
	(info.memberType.push_back(ExtractTypeName<std::remove_cv_t<std::remove_reference_t<Args>>>()), ...);
	if constexpr (std::is_void_v<ReturnType>)
	{
		info.retrunType = "void";
		info.invoker = [Func](void* target, std::vector<void*>& argList) -> PValue
			{
				auto k = std::make_index_sequence<sizeof...(Args)>{};
				ClassType* C = static_cast<ClassType*>(target);
				CallHelper(C, Func, argList, k);
				return PValue();
			};
	}
	else
	{
		info.retrunType = GetCleanTypeName<ReturnType>();
		info.invoker = [Func](void* target, std::vector<void*>& argList) -> PValue
			{
				auto k = std::make_index_sequence<sizeof...(Args)>{};
				ClassType* C = static_cast<ClassType*>(target);
				PValue temp = PValue(CallHelperReturn(C, Func, argList, k));
				return temp;
			};
	}
	return info;
}

template<typename TClass, typename... Args, size_t... Is>
static void CallHelper(TClass* obj, void(TClass::* func)(Args...), std::vector<void*>& args, std::index_sequence<Is...>)
{
	if constexpr (sizeof...(Args) == 0)
	{
		(obj->*func)();
	}
	else
	{
		(obj->*func)((*static_cast<std::remove_reference_t<Args>*>(args[Is]))...);
	}
}
template<typename TClass, typename TReturn, typename... Args, size_t... Is>
static TReturn CallHelperReturn(TClass* obj, TReturn(TClass::* func)(Args...), std::vector<void*>& args, std::index_sequence<Is...>)
{
	if constexpr (sizeof...(Args) == 0)
	{
		TReturn temp = (obj->*func)();
		return temp;
	}
	else
	{
		TReturn temp = (obj->*func)((*static_cast<std::remove_reference_t<Args>*>(args[Is]))...);
		return temp;
	}
}
