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
	std::string returnType;
	std::string classType;
	std::vector<std::string> memberType;
	std::function<PValue (void*, std::vector<void*>&)> invoker;
};

///
extern "C" PIXEL_META_DLL uint64_t StringToByHash(const char* name);

class PNamespace;
class PClass;
using C_string = const std::string&;

///생성
extern "C" PIXEL_META_DLL PNamespace*	CreateNewNamepace(const std::string& Name);
extern "C" PIXEL_META_DLL PClass*		CreateNewClass(const std::string& thidTypeName, const std::string& parentTypeName = "");

///GetType
extern "C" PIXEL_META_DLL PType*		GetType(const std::string& name);
extern "C" PIXEL_META_DLL PClass*		GetClass(const std::string& name);
extern "C" PIXEL_META_DLL PNamespace*	GetNamespace(const std::string& name);

///모든 Type을 순회 하는 함수들
extern "C" PIXEL_META_DLL int			GetTypeAllCount();
extern "C" PIXEL_META_DLL PType*		GetTypeByIndex(int index);
extern "C" PIXEL_META_DLL C_string		GetTypeName(PType* target);
extern "C" PIXEL_META_DLL META_TYPE		GetTypeCategory(PType* type);


///클래스 전용 함수들
//생성자 소멸자
extern "C" PIXEL_META_DLL void		CreateClassFunction(PClass* targetClass,void*(createFunc)());
extern "C" PIXEL_META_DLL void		DeleteClassFunction(PClass* targetClass,void(deleteFunc)());
extern "C" PIXEL_META_DLL void		CastClassFunction(PClass* targetClass, void*(castFunc)());
//타입정보
extern "C" PIXEL_META_DLL PClass*	GetClassByIndex(int index);
extern "C" PIXEL_META_DLL C_string	GetClassTypeName(PClass* targetClass);
extern "C" PIXEL_META_DLL uint64_t	GetClassHash(PClass* targetClass);
extern "C" PIXEL_META_DLL uint64_t	GetClassHashByString(const std::string& className);
extern "C" PIXEL_META_DLL uint64_t	GetClassParentHash(PClass* targetClass);
extern "C" PIXEL_META_DLL uint64_t	GetClassChildHash(PClass* targetClass);
//등록
extern "C" PIXEL_META_DLL bool		AddMember(PClass* targetClass, const std::string& memberName, MemberInfo info,long flag = 0);
extern "C" PIXEL_META_DLL bool		AddMethod(PClass* targetClass,const std::string& methodName, MethodInfo info, long flag = 0);
//맴버 정보
extern "C" PIXEL_META_DLL C_string	GetClassMemberName(PClass* targetClass,int index);
extern "C" PIXEL_META_DLL C_string	GetClassMemberType(PClass* targetClass,int index);
extern "C" PIXEL_META_DLL bool		HasClassMemberFlag(PClass* targetClass,int index,long flag);
extern "C" PIXEL_META_DLL void*		GetClassMemberValue(PClass* targetClass,int index,void* target);
extern "C" PIXEL_META_DLL void		SetClassMemberValue(PClass* targetClass,int index,void* target,void* value);
extern "C" PIXEL_META_DLL int		GetClassMemberCount(PClass* targetClass);
//함수 정보
extern "C" PIXEL_META_DLL C_string	GetClassMethodName(PClass* targetClass, int index);
extern "C" PIXEL_META_DLL C_string	GetClassMethodReturnType(PClass* targetClass, int index);
extern "C" PIXEL_META_DLL C_string	GetClassMethodGetPropertyType(PClass* targetClass, int index,int propertyIndex);
extern "C" PIXEL_META_DLL bool		HasClassMethodFlag(PClass* targetClass, int index, long flag);
extern "C" PIXEL_META_DLL int		GetClassMethodPropertyCount(PClass* targetClass, int index);
extern "C" PIXEL_META_DLL int		GetClassMethodCount(PClass* targetClass);
//콜
extern "C" PIXEL_META_DLL PValue	CallClassMethod(PClass* targetClass,int index,void* target,std::vector<void*> property);


///글로벌 Namespace
extern "C" PIXEL_META_DLL C_string	GetNamespaceMethodName(PNamespace* target, int index);
extern "C" PIXEL_META_DLL C_string	GetNamespaceMethodReturnType(PNamespace* target, int index);
extern "C" PIXEL_META_DLL C_string	GetNamespaceMethodGetPropertyType(PNamespace* target, int index, int propertyIndex);
extern "C" PIXEL_META_DLL bool		HasNamespaceMethodFlag(PNamespace* target, int index, long flag);
extern "C" PIXEL_META_DLL int		GetNamespaceMethodPropertyCount(PNamespace* target, int index);
extern "C" PIXEL_META_DLL int		GetNamespaceMethodCount(PNamespace* target);














extern "C" PIXEL_META_DLL bool AddGlobalMethod(PNamespace* targetClass, const std::string& methodName, MethodInfo info,long flag = 0);

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

template<typename TClass, typename... Args, size_t... Is>
static void CallHelper(TClass* obj, void(TClass::* func)(Args...), std::vector<void*>& args, std::index_sequence<Is...>)
{
	(obj->*func)((*static_cast<std::remove_reference_t<Args>*>(args[Is]))...);
}
template<typename TClass, typename TReturn, typename... Args, size_t... Is>
static TReturn CallHelperReturn(TClass* obj, TReturn(TClass::* func)(Args...), std::vector<void*>& args, std::index_sequence<Is...>)
{
	return (obj->*func)((*static_cast<std::remove_reference_t<Args>*>(args[Is]))...);
}
template<typename... Args, size_t... Is>
static void GlobalCallHelper(void(*func)(Args...), std::vector<void*>& args, std::index_sequence<Is...>)
{
	(func)((*static_cast<std::remove_reference_t<Args>*>(args[Is]))...);
}
template<typename TReturn, typename... Args, size_t... Is>
static TReturn GlobalCallHelperReturn(TReturn(*func)(Args...), std::vector<void*>& args, std::index_sequence<Is...>)
{
	return  (func)((*static_cast<std::remove_reference_t<Args>*>(args[Is]))...);
}


template<typename ClassType, typename ReturnType, typename... Args>
MethodInfo GetMethodInfo(ReturnType(ClassType::*Func)(Args...))
{
	MethodInfo info;

	info.classType = ExtractTypeName<ClassType>();
	(info.memberType.push_back(ExtractTypeName<std::remove_cv_t<std::remove_reference_t<Args>>>()), ...);
	if constexpr (std::is_void_v<ReturnType>)
	{
		info.returnType = "void";
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
		info.returnType = GetCleanTypeName<ReturnType>();
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

template<typename ReturnType, typename... Args>
MethodInfo GeGlobalMethodInfo(ReturnType(*Func)(Args...))
{
	MethodInfo info;
	(info.memberType.push_back(ExtractTypeName<std::remove_cv_t<std::remove_reference_t<Args>>>()), ...);

	if constexpr (std::is_void_v<ReturnType>)
	{
		info.returnType = "void";

		// 3. [경고 방지] 사용하지 않는 target은 주석 처리하여 Warning(C4100 등) 방지
		info.invoker = [Func](void* /*target*/, std::vector<void*>& argList) -> PValue
			{
				auto k = std::make_index_sequence<sizeof...(Args)>{};
				GlobalCallHelper(Func, argList, k);
				return PValue();
			};
	}
	else
	{
		info.returnType = GetCleanTypeName<ReturnType>(); // 2. [오타 수정] retrunType -> returnType

		info.invoker = [Func](void* /*target*/, std::vector<void*>& argList) -> PValue
			{
				auto k = std::make_index_sequence<sizeof...(Args)>{};
				return PValue(GlobalCallHelperReturn(Func, argList, k));
			};
	}
	return info;
}

