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
	std::vector<std::string> memberName;
	std::vector<std::string> memberTooltip;
	std::string tooltip;
	std::function<PValue (void*, std::vector<void*>&)> invoker;
};


inline MethodInfo SetMethodTooltip(MethodInfo info, const std::string& tooltip)
{
	info.tooltip = tooltip;
	return info;
}

inline MethodInfo SetMethodParameterName(MethodInfo info, int index, const std::string& name)
{
	if (index < 0) return info;
	size_t parameterIndex = static_cast<size_t>(index);
	if (info.memberName.size() < info.memberType.size()) info.memberName.resize(info.memberType.size());
	if (parameterIndex >= info.memberName.size()) info.memberName.resize(parameterIndex + 1);
	info.memberName[parameterIndex] = name;
	return info;
}

inline MethodInfo SetMethodParameterTooltip(MethodInfo info, int index, const std::string& tooltip)
{
	if (index < 0) return info;
	size_t parameterIndex = static_cast<size_t>(index);
	if (info.memberTooltip.size() < info.memberType.size()) info.memberTooltip.resize(info.memberType.size());
	if (parameterIndex >= info.memberTooltip.size()) info.memberTooltip.resize(parameterIndex + 1);
	info.memberTooltip[parameterIndex] = tooltip;
	return info;
}
///
extern "C" PIXEL_META_DLL uint64_t StringToByHash(const char* name);

class PType;
class PStatic;
class PClass;
class PEnum;
using C_string = const std::string&;

///생성
extern "C" PIXEL_META_DLL PStatic*		CreateNewStatic(const std::string& Name);
extern "C" PIXEL_META_DLL PClass*		CreateNewClass(const std::string& thidTypeName, const std::string& parentTypeName = "");
extern "C" PIXEL_META_DLL PEnum*		CreateNewEnum(const std::string& thidTypeName);

///GetType
extern "C" PIXEL_META_DLL int			GetTypeAllCount();
extern "C" PIXEL_META_DLL PType*		GetType(const std::string& name);
extern "C" PIXEL_META_DLL PType*		GetTypeByIndex(int index);
extern "C" PIXEL_META_DLL C_string		GetTypeName(PType* target);
extern "C" PIXEL_META_DLL META_TYPE		GetTypeCategory(PType* type);
extern "C" PIXEL_META_DLL uint64_t		GetTypeParentByHash(PType* target);
extern "C" PIXEL_META_DLL uint64_t		GetTypeHash(PType* type);
extern "C" PIXEL_META_DLL uint64_t		GetTypeHashByName(const std::string& name);
extern "C" PIXEL_META_DLL void			SetTypeFlag(PType* type, long flag);
extern "C" PIXEL_META_DLL long			GetTypeFlag(PType* type);
extern "C" PIXEL_META_DLL bool			HasTypeFlag(PType* type, long flag);

extern "C" PIXEL_META_DLL int			GetMemberCount(PType* type);
extern "C" PIXEL_META_DLL C_string		GetMemberType(PType* type, int index);
extern "C" PIXEL_META_DLL C_string		GetMemberName(PType* type, int index);
extern "C" PIXEL_META_DLL long			GetMemberFlag(PType* type, int index);
extern "C" PIXEL_META_DLL bool			HasMemberFlag(PType* type, int index, long flag);

extern "C" PIXEL_META_DLL int			GetMethodCount(PType* type);
extern "C" PIXEL_META_DLL C_string		GetMethodName(PType* type, int index);
extern "C" PIXEL_META_DLL C_string		GetMethodReturnType(PType* type, int index);
extern "C" PIXEL_META_DLL C_string		GetMethodTooltip(PType* type, int index);
extern "C" PIXEL_META_DLL int			GetMethodPropertyCount(PType* type,int index);
extern "C" PIXEL_META_DLL C_string		GetMethodGetPropertyType(PType* type, int index, int propertyIndex);
extern "C" PIXEL_META_DLL C_string		GetMethodGetPropertyName(PType* type, int index, int propertyIndex);
extern "C" PIXEL_META_DLL C_string		GetMethodGetPropertyTooltip(PType* type, int index, int propertyIndex);
extern "C" PIXEL_META_DLL bool			HasMethodFlag(PType* type, int index, long flag);
extern "C" PIXEL_META_DLL PValue		CallMethod(PType* targetClass, int index, void* target, std::vector<void*> property);

extern "C" PIXEL_META_DLL int			GetEnumCount(PType* type);
extern "C" PIXEL_META_DLL C_string		GetEnum(PType* type,int index);

///클래스 전용 함수들
//생성자 소멸자
extern "C" PIXEL_META_DLL void		CreateClassFunction(PClass* targetClass,void*(createFunc)());
extern "C" PIXEL_META_DLL void		DeleteClassFunction(PClass* targetClass,void(deleteFunc)());
extern "C" PIXEL_META_DLL void		CastClassFunction(PClass* targetClass, void*(castFunc)());

////등록
extern "C" PIXEL_META_DLL bool		AddMember(PClass* targetClass, const std::string& memberName, MemberInfo info,long flag = 0);
extern "C" PIXEL_META_DLL bool		AddMethod(PClass* targetClass,const std::string& methodName, MethodInfo info, long flag = 0);
extern "C" PIXEL_META_DLL bool		AddEnum(PEnum* target,const std::string& Key);
















extern "C" PIXEL_META_DLL bool AddGlobalMethod(PStatic* targetClass, const std::string& methodName, MethodInfo info,long flag = 0);

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


template <typename Signature, typename ClassType>
constexpr auto Resolve(Signature ClassType::* func)
{
	return func;
}

#define OVERLOAD(ReturnType, ClassName, MethodName, ...) \
    static_cast<ReturnType(ClassName::*)(__VA_ARGS__)>(&ClassName::MethodName)

template<typename ReturnType, typename... Args>
MethodInfo GetMethodInfo(ReturnType(*Func)(Args...))
{
	MethodInfo info;

	info.classType = "Lambda"; // 클래스가 없으므로 임의 지정
	(info.memberType.push_back(ExtractTypeName<std::remove_cv_t<std::remove_reference_t<Args>>>()), ...);

	if constexpr (std::is_void_v<ReturnType>)
	{
		info.returnType = "void";
		info.invoker = [Func](void* target, std::vector<void*>& argList) -> PValue
			{
				auto k = std::make_index_sequence<sizeof...(Args)>{};
				// 멤버 함수가 아니므로 target(클래스 포인터) 없이 Func를 직접 호출하는 CallHelperGlobal이 필요합니다.
				GlobalCallHelper(Func, argList, k);
				return PValue();
			};
	}
	else
	{
		info.returnType = GetCleanTypeName<ReturnType>();
		info.invoker = [Func](void* target, std::vector<void*>& argList) -> PValue
			{
				auto k = std::make_index_sequence<sizeof...(Args)>{};
				PValue temp = PValue(GlobalCallHelperReturn(Func, argList, k));
				return temp;
			};
	}
	return info;
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

