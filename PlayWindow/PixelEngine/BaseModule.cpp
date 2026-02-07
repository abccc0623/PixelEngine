#include "BaseModule.h"
#include <type_traits>
BaseModule::BaseModule()
{
	hasCode = std::hash<BaseModule*>{}(this);
}

BaseModule::~BaseModule()
{

}

size_t BaseModule::GetHashCode()
{
	return hasCode;
}
