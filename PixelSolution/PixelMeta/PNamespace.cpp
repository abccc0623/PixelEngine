#include "pch.h"
#include "PNamespace.h"
#include "PMethod.h"
PNamespace::PNamespace(uint64_t hash, std::string name, size_t size):
	PType(hash, name, size)
{
	metaType = META_TYPE::NAMESPACE;
}

PNamespace::~PNamespace()
{
	for (auto K : methodList)
	{
		delete K;
	}
	methodList.clear();
	methodMap.clear();
}

int PNamespace::GetMethodCount()
{
	return methodList.size();
}

int PNamespace::GetPropertyCount(int index)
{
	return methodList[index]->GetPropertyCount();
}

bool PNamespace::HasClassMethodFlag(int index, long flag)
{
	return methodList[index]->HasFlag(flag);
}

const std::string& PNamespace::GetMethodName(int index)
{
	return methodList[index]->GetName();
}

const std::string& PNamespace::GetMethodReturnType(int index)
{
	return methodList[index]->GetReturnType();
}

const std::string& PNamespace::GetMethodPropertyType(int index, int propertyIndex)
{
	return methodList[index]->GetPropertyType(propertyIndex);
}

PValue PNamespace::CallMethod(int index, void* target, std::vector<void*> property)
{
	return PValue();
}

void PNamespace::AddMethod(PMethod* method)
{
	methodMap.insert({ method->GetTypeHash(),method });
	methodList.push_back(method);
}
