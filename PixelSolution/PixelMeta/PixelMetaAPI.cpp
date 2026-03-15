#include "pch.h"
#include "PixelMetaAPI.h"
#include "PType.h"
#include "PClass.h"
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
	if (System == nullptr){System = new RSystem();}
	System->Register(temp);
	AddMethod(temp, "Create", GeMethodInfo(&PClass::CallCreateFunction));
	AddMethod(temp, "Delete", GeMethodInfo(&PClass::CallDeleteFunction));
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
	if (System == nullptr) { System = new RSystem(); }
	return System->VectorByHash.size();
}

PClass* GetClassByIndex(int index)
{
	if (System == nullptr) { System = new RSystem(); }
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

PClass* GetClass(const std::string& className)
{
	if (System == nullptr) { System = new RSystem(); }
	uint64_t thisTypeHash = HashUtil::ConstexprHash(className.c_str());
	PType* type = System->GetType(thisTypeHash);
	if (type->GetMetaType() == (int)META_TYPE::CLASS)
	{
		return reinterpret_cast<PClass*>(type);
	}
	return nullptr;
}

uint64_t GetClassHashByString(const std::string& className)
{
	if (System == nullptr) { System = new RSystem(); }
	uint64_t thisTypeHash = HashUtil::ConstexprHash(className.c_str());
	PType* type = System->GetType(thisTypeHash);
	if (type->GetMetaType() == (int)META_TYPE::CLASS)
	{
		return type->GetHash();
	}
	return thisTypeHash;
}

uint64_t GetClassHash(PClass* targetClass)
{
	if (System == nullptr) { System = new RSystem(); }
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

const std::string& GetClassMethodGetPropertyType(PClass* targetClass, int index, int propertyIndex)
{
	return targetClass->GetMethodPropertyType(index, propertyIndex);
}

const std::string& GetClassMemberName(PClass* targetClass, int index)
{
	return targetClass->GetMemberName(index);
}

const std::string& GetClassMemberType(PClass* targetClass, int index)
{
	return targetClass->GetMemberType(index);
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
	if (System == nullptr) { System = new RSystem(); }
	System->Release();
	delete System;
	System = nullptr;
}

int GetClassMethodCount(PClass* targetClass)
{
	return targetClass->GetMethodCount();
}

bool AddMember(PClass* targetClass, const std::string& memberName, MemberInfo info)
{
	uint64_t nameHash = HashUtil::ConstexprHash(memberName.c_str());
	if (System == nullptr) { System = new RSystem(); }
	PField* field = new PField(System->GetTypeByString(info.memberType), memberName, info.offset);
	targetClass->AddField(field);
	return true;
}

bool AddMethod(PClass* targetClass, const std::string& methodName, MethodInfo info)
{
	uint64_t nameHash = HashUtil::ConstexprHash(methodName.c_str());
	if (System == nullptr) { System = new RSystem(); }
	PMethod* field = new PMethod(methodName);
	field->SetInfo(info.retrunType,info.classType, info.memberType,info.invoker);
	targetClass->AddMethod(field);
	return false;
}
