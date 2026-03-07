#include "pch.h"
#include "PMethod.h"
#include "PType.h"

PMethod::PMethod(std::string name, PType* returnType, std::vector<PType*> memberList):
	methodName(name), memberReturn(returnType), memberList(memberList)
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

PValue PMethod::Invoke(void* instance, std::vector<void*> members)
{
	return templateFunction(instance, members);;
}
