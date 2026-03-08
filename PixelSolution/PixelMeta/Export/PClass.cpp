#include "pch.h"
#include "PClass.h"
#include "PField.h"

static std::string G_NONE_STRING = "NONE";
PClass::PClass():
	parent(nullptr),
	child(nullptr)
{
	typeHash = 0;
	typeName = "";
	typeSize = 0;
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

int PClass::GetFieldMaxCount()
{
	return memberList.size();
}

int PClass::GetMethodMaxCount()
{
	return methodList.size();
}

const std::string& PClass::TypeName()
{
	return typeName;
}

const std::string& PClass::ParentName()
{
	if (parent != nullptr)
	{
		return parent->typeName;
	}
	return G_NONE_STRING;
}

const std::string& PClass::ChildName()
{
	if (child != nullptr)
	{
		return child->typeName;
	}
	return G_NONE_STRING;
}

const std::string& PClass::GetFieldName(int index)
{
	return memberList[index]->GetName();
}

const std::string& PClass::GetFieldType(int index)
{
	return memberList[index]->GetType();
}
