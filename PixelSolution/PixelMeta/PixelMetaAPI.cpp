#include "pch.h"
#include "PixelMetaAPI.h"
#include "PType.h"
#include "PClass.h"
#include "PNamespace.h"
#include "RSystem.h"
#include "GlobalHashCode.h"
#include "PField.h"
#include "PMethod.h"

RSystem* System = nullptr;
uint64_t StringToByHash(const char* name)
{
	std::string Str(name);
	uint64_t thisTypeHash = HashUtil::ConstexprHash(Str.c_str());
	return thisTypeHash;
}

PNamespace* CreateNewNamepace(const std::string& Name)
{
	if (Name.empty()) return nullptr;
	uint64_t thisTypeHash = HashUtil::ConstexprHash(Name.c_str());
	PNamespace* temp = new PNamespace(thisTypeHash, Name, 0);
	RSystem::GetInstance().Register(temp);
	return temp;
}

PClass* CreateNewClass(const std::string& thidTypeName, const std::string& parentTypeName)
{
	if (thidTypeName.empty()) return nullptr;
	uint64_t thisTypeHash = HashUtil::ConstexprHash(thidTypeName.c_str());
	PClass* temp = new PClass(thisTypeHash, thidTypeName,0);
	if (!parentTypeName.empty())
	{
		uint64_t parentTypeHash = 0;
		parentTypeHash = HashUtil::ConstexprHash(parentTypeName.c_str());
		temp->SetParentHash(parentTypeHash);
	}
	RSystem::GetInstance().Register(temp);
	AddMethod(temp, "Create", GetMethodInfo(&PClass::CallCreateFunction));
	AddMethod(temp, "Delete", GetMethodInfo(&PClass::CallDeleteFunction));
	return temp;
}

void CreateClassFunction(PClass* targetClass, void* (createFunc)())
{
	targetClass->SetCreateFunction(createFunc);
}

void DeleteClassFunction(PClass* targetClass, void (deleteFunc)())
{
	targetClass->SetDeleteFunction(deleteFunc);
}

void CastClassFunction(PClass* targetClass, void* (castFunc)())
{

}

int AllClassCount()
{
	return System->VectorByHash.size();
}

PClass* GetClassByIndex(int index)
{
	PType* type = System->VectorByHash[index];
	if (type->GetMetaType() == (int)META_TYPE::CLASS)
	{
		return reinterpret_cast<PClass*>(type);
	}
	else
	{
		return nullptr;
	}
}

const std::string& GetClassTypeName(PClass* targetClass)
{
	return targetClass->GetName();
}

PType* GetType(const std::string& typeName)
{
	uint64_t thisTypeHash = HashUtil::ConstexprHash(typeName.c_str());
	PType* type = System->GetType(thisTypeHash);
	return type;
}

PClass* GetClass(const std::string& className)
{
	uint64_t thisTypeHash = HashUtil::ConstexprHash(className.c_str());
	PType* type = RSystem::GetInstance().GetType(thisTypeHash);
	if (type->GetMetaType() == (int)META_TYPE::CLASS)
	{
		return reinterpret_cast<PClass*>(type);
	}
	return nullptr;
}

PNamespace* GetNamespace(const std::string& name)
{
	uint64_t thisTypeHash = HashUtil::ConstexprHash(name.c_str());
	PType* type = RSystem::GetInstance().GetType(thisTypeHash);
	if (type->GetMetaType() == (int)META_TYPE::NAMESPACE)
	{
		return reinterpret_cast<PNamespace*>(type);
	}
	return nullptr;
}


int GetTypeAllCount()
{
	return RSystem::GetInstance().GetTypeAllCount();
}

PType* GetTypeByIndex(int index)
{
	return RSystem::GetInstance().GetTypeByIndex(index);
}

C_string GetTypeName(PType* target)
{
	return target->GetName();
}

META_TYPE GetTypeCategory(PType* type)
{
	return (META_TYPE)type->GetMetaType();
}


uint64_t GetClassHashByString(const std::string& className)
{
	uint64_t thisTypeHash = HashUtil::ConstexprHash(className.c_str());
	PType* type = RSystem::GetInstance().GetType(thisTypeHash);
	if (type->GetMetaType() == (int)META_TYPE::CLASS)
	{
		return type->GetHash();
	}
	return thisTypeHash;
}

uint64_t GetClassHash(PClass* targetClass)
{
	return targetClass->GetHash();
}

uint64_t GetClassParentHash(PClass* targetClass)
{
	return targetClass->GetParentHash();
}

uint64_t GetClassChildHash(PClass* targetClass)
{
	return targetClass->GetChildHash();
}

int GetClassMemberCount(PClass* targetClass)
{
	return targetClass->GetMemberCount();
}

const std::string& GetClassMethodName(PClass* targetClass, int index)
{
	return targetClass->GetMethodName(index);
}

const std::string& GetClassMethodReturnType(PClass* targetClass, int index)
{
	return targetClass->GetMethodReturnType(index);
}

int GetClassMethodPropertyCount(PClass* targetClass, int index)
{
	return targetClass->GetPropertyCount(index);
}

PValue CallClassMethod(PClass* targetClass, int index, void* target, std::vector<void*> property)
{
	return targetClass->CallMethod(index, target, property);
}

C_string GetNamespaceMethodName(PNamespace* target, int index)
{
	return target->GetMethodName(index);
}

C_string GetNamespaceMethodReturnType(PNamespace* target, int index)
{
	return target->GetMethodReturnType(index);
}

C_string GetNamespaceMethodGetPropertyType(PNamespace* target, int index, int propertyIndex)
{
	return target->GetMethodPropertyType(index, propertyIndex);
}

bool HasNamespaceMethodFlag(PNamespace* target, int index, long flag)
{
	return target->HasClassMethodFlag(index,flag);
}

int GetNamespaceMethodPropertyCount(PNamespace* target, int index)
{
	return target->GetPropertyCount(index);
}

int GetNamespaceMethodCount(PNamespace* target)
{
	return target->GetMethodCount();
}

bool AddGlobalMethod(PNamespace* targetClass, const std::string& methodName, MethodInfo info, long flag)
{
	uint64_t nameHash = HashUtil::ConstexprHash(methodName.c_str());
	PMethod* field = new PMethod(methodName);
	field->SetInfo(info.returnType, info.classType, info.memberType, info.invoker);
	field->SetFlag(flag);
	targetClass->AddMethod(field);
	return true;
}

const std::string& GetClassMethodGetPropertyType(PClass* targetClass, int index, int propertyIndex)
{
	return targetClass->GetMethodPropertyType(index, propertyIndex);
}

bool HasClassMethodFlag(PClass* targetClass, int index, long flag)
{
	return targetClass->HasClassMethodFlag(index, flag);
}

const std::string& GetClassMemberName(PClass* targetClass, int index)
{
	return targetClass->GetMemberName(index);
}

const std::string& GetClassMemberType(PClass* targetClass, int index)
{
	return targetClass->GetMemberType(index);
}

bool HasClassMemberFlag(PClass* targetClass, int index, long flag)
{
	return targetClass->HasClassMemberFlag(index, flag);
}

void* GetClassMemberValue(PClass* targetClass, int index, void* target)
{
	return targetClass->GetMemberValue(index, target);
}

void SetClassMemberValue(PClass* targetClass, int index, void* target, void* value)
{
	targetClass->SetMemberValue(index, target, value);
}

void ReleaseMetaType()
{
	RSystem::GetInstance().Release();
}

int GetClassMethodCount(PClass* targetClass)
{
	return targetClass->GetMethodCount();
}

bool AddMember(PClass* targetClass, const std::string& memberName, MemberInfo info, long flag)
{
	uint64_t nameHash = HashUtil::ConstexprHash(memberName.c_str());
	PField* field = new PField(RSystem::GetInstance().GetTypeByString(info.memberType), memberName, info.offset);
	field->SetFlag(flag);
	targetClass->AddField(field);
	return true;
}

bool AddMethod(PClass* targetClass, const std::string& methodName, MethodInfo info, long flag)
{
	uint64_t nameHash = HashUtil::ConstexprHash(methodName.c_str());
	PMethod* field = new PMethod(methodName);
	field->SetInfo(info.returnType,info.classType, info.memberType,info.invoker);
	field->SetFlag(flag);
	targetClass->AddMethod(field);
	return false;
}
