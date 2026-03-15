#include "pch.h"
#include "PMethod.h"
#include "PType.h"
#include "GlobalHashCode.h"

PMethod::PMethod(std::string name):
	methodName(name)
{
	typeHash = HashUtil::ConstexprHash(name.c_str());
}

PMethod::~PMethod()
{

}

const std::string& PMethod::GetName()
{
	return methodName;
}

const std::string& PMethod::GetReturnType()
{
	return retrunType;
}

const std::string& PMethod::GetPropertyType(int index)
{
	return propertyType[index];
}

int PMethod::GetPropertyCount()
{
	return propertyType.size();
}

PValue PMethod::Call(void* target, std::vector<void*> property)
{
	if (invoker != nullptr)
	{
		return invoker(target, property);
	}
	return PValue();
}

uint64_t PMethod::GetTypeHash()
{
	return typeHash;
}

void PMethod::SetInfo(std::string retrunType, std::string classType, std::vector<std::string> memberType, std::function<PValue(void*, std::vector<void*>&)> func)
{
	this->retrunType = retrunType;
	this->classType = classType;
	this->propertyType = memberType;
	this->invoker = func;
}

