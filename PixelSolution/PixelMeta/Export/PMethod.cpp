#include "pch.h"
#include "PMethod.h"
#include "PType.h"

PMethod::PMethod(std::string name):
	methodName(name)
{

}

PMethod::~PMethod()
{

}

void* PMethod::operator new(size_t size)
{
	return ::malloc(size);
}

void PMethod::operator delete(void* ptr)
{
	::free(ptr);
}

PValue PMethod::Invoke(void* instance, std::vector<void*>& members)
{
	if(NoReturnFunction)
	{
		NoReturnFunction(instance, members);
	}
	return PValue();
}
