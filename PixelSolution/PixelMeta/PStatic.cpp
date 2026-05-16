#include "pch.h"
#include "PStatic.h"
#include "PMethod.h"
PStatic::PStatic(uint64_t hash, std::string name, size_t size) :
	PType(hash, name, size)
{
	metaType = META_TYPE::STATIC;
}

PStatic::~PStatic()
{
	for (auto K : methodList)
	{
		delete K;
	}
	methodList.clear();
	methodMap.clear();
}

int PStatic::GetMethodCount()
{
	return methodList.size();
}

int PStatic::GetPropertyCount(int index)
{
	return methodList[index]->GetPropertyCount();
}

bool PStatic::HasClassMethodFlag(int index, long flag)
{
	return methodList[index]->HasFlag(flag);
}

const std::string& PStatic::GetMethodName(int index)
{
	return methodList[index]->GetName();
}

const std::string& PStatic::GetMethodReturnType(int index)
{
	return methodList[index]->GetReturnType();
}

const std::string& PStatic::GetMethodPropertyType(int index, int propertyIndex)
{
	return methodList[index]->GetPropertyType(propertyIndex);
}

PValue PStatic::CallMethod(int index, void* target, std::vector<void*> property)
{
	return methodList[index]->Call(nullptr, property);
}

void PStatic::AddMethod(PMethod* method)
{
	methodMap.insert({ method->GetTypeHash(),method });
	methodList.push_back(method);
}
