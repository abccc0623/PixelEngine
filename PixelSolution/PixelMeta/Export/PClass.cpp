#include "pch.h"
#include "PClass.h"
#include "PField.h"
#include "PMethod.h"

static std::string G_NONE_STRING = "NONE";
PClass::PClass(uint64_t hash, std::string name, size_t size):
	PType(hash, name, size),
	parentHash(0),
	childHash(0)
{
	metaType = META_TYPE::CLASS;
}

PClass::~PClass()
{
	for (auto K : memberList)
	{
		delete K;
	}
	
	for (auto K : methodList)
	{
		delete K;
	}
	memberList.clear();
	methodList.clear();
	memberMap.clear();
	methodMap.clear();
}

void PClass::AddField(PField* field)
{
	memberMap.insert({field->GetTypeHash(),field });
	memberList.push_back(field);
}

void PClass::AddMethod(PMethod* method)
{
	methodMap.insert({ method->GetTypeHash(),method });
	methodList.push_back(method);
}

uint64_t PClass::GetParentHash()
{
	return parentHash;
}

uint64_t PClass::GetChildHash()
{
	return childHash;
}

void PClass::SetParentHash(uint64_t hash)
{
	parentHash = hash;
}

void PClass::SetChildHash(uint64_t hash)
{
	childHash = hash;
}

int PClass::GetMemberCount()
{
	return memberList.size();
}

const std::string& PClass::GetMemberName(int index)
{
	return memberList[index]->GetName();
}

const std::string& PClass::GetMemberType(int index)
{
	return memberList[index]->GetType();
}

long PClass::GetClassMemberFlag(int index)
{
	return memberList[index]->GetFlag();
}

bool PClass::HasClassMemberFlag(int index, long flag)
{
	return memberList[index]->HasFlag(flag);
}

void* PClass::GetMemberValue(int index, void* target)
{
	return  memberList[index]->GetValue(target);
}

void PClass::SetMemberValue(int index, void* target, void* value)
{
	memberList[index]->SetValue(target, value);
}

int PClass::GetMethodCount()
{
	return methodList.size();
}

int PClass::GetPropertyCount(int index)
{
	return methodList[index]->GetPropertyCount();
}

bool PClass::HasClassMethodFlag(int index, long flag)
{
	return methodList[index]->HasFlag(flag);
}

const std::string& PClass::GetMethodName(int index)
{
	return methodList[index]->GetName();
}

const std::string& PClass::GetMethodReturnType(int index)
{
	return methodList[index]->GetReturnType();
}

const std::string& PClass::GetMethodTooltip(int index)
{
	return methodList[index]->GetTooltip();
}

const std::string& PClass::GetMethodPropertyType(int index, int propertyIndex)
{
	return methodList[index]->GetPropertyType(propertyIndex);
}

const std::string& PClass::GetMethodPropertyName(int index, int propertyIndex)
{
	return methodList[index]->GetPropertyName(propertyIndex);
}

const std::string& PClass::GetMethodPropertyTooltip(int index, int propertyIndex)
{
	return methodList[index]->GetPropertyTooltip(propertyIndex);
}

PValue PClass::CallMethod(int index, void* target, std::vector<void*> property)
{
	return methodList[index]->Call(target, property);
}

void PClass::SetCreateFunction(CreateFunction func)
{
	createFunc = func;
}

void PClass::SetDeleteFunction(DeleteFunction func)
{
	deleteFunc = func;
}

void PClass::SetCastFunction(TypeCastFunction func)
{
	castFunc = func;
}

void* PClass::CallCreateFunction()
{
	if (createFunc != nullptr)
	{
		return createFunc();
	}
	return nullptr;
}

void PClass::CallDeleteFunction()
{
	if (deleteFunc != nullptr)
	{
		deleteFunc();
	}
}

