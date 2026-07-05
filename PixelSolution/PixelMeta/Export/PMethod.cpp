#include "pch.h"
#include "PMethod.h"
#include "PType.h"
#include "GlobalHashCode.h"

static std::string G_EMPTY_STRING = "";

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

const std::string& PMethod::GetTooltip()
{
	return tooltip;
}

const std::string& PMethod::GetPropertyType(int index)
{
	if (index < 0) return G_EMPTY_STRING;
	size_t propertyIndex = static_cast<size_t>(index);
	if (propertyIndex >= propertyType.size()) return G_EMPTY_STRING;
	return propertyType[propertyIndex];
}

const std::string& PMethod::GetPropertyName(int index)
{
	if (index < 0) return G_EMPTY_STRING;
	size_t propertyIndex = static_cast<size_t>(index);
	if (propertyIndex >= propertyName.size()) return G_EMPTY_STRING;
	return propertyName[propertyIndex];
}

const std::string& PMethod::GetPropertyTooltip(int index)
{
	if (index < 0) return G_EMPTY_STRING;
	size_t propertyIndex = static_cast<size_t>(index);
	if (propertyIndex >= propertyTooltip.size()) return G_EMPTY_STRING;
	return propertyTooltip[propertyIndex];
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

void PMethod::SetFlag(long flag)
{
	Flag.Set(flag);
}

bool PMethod::HasFlag(long flag)
{
	return Flag.Has(flag);
}

uint64_t PMethod::GetTypeHash()
{
	return typeHash;
}

void PMethod::SetInfo(std::string retrunType, std::string classType, std::vector<std::string> memberType, std::vector<std::string> memberName, std::vector<std::string> memberTooltip, std::string tooltip, std::function<PValue(void*, std::vector<void*>&)> func)
{
	this->retrunType = retrunType;
	this->classType = classType;
	this->propertyType = memberType;
	this->propertyName = memberName;
	this->propertyTooltip = memberTooltip;
	this->tooltip = tooltip;
	this->invoker = func;
}

