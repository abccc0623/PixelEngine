#include "pch.h"
#include "PType.h"
#include "PField.h"
#include "PMethod.h"

PType::PType(uint64_t Hash, std::string name, size_t size):
	typeHash(Hash), typeName(name), typeSize(size)
{

}
PType::PType():
	typeHash(0), typeName(""), typeSize(0)
{

}
PType::~PType()
{
}
std::string PType::GetName()
{
	return typeName;
}
uint64_t PType::GetHash()
{
	return typeHash;
}
uint64_t PType::GetSize()
{
	return typeSize;
}

PField* PType::GetField(const std::string& name)
{
	return nullptr;
}

PMethod* PType::GetMethod(const std::string& name)
{
	return nullptr;
}

bool PType::operator==(const PType& other)
{
	return (this->typeHash == other.typeHash);
}

bool PType::operator!=(const PType& other)
{
	return (this->typeHash != other.typeHash);
}

void* PType::operator new(size_t size)
{
	return ::malloc(size);
}

void PType::operator delete(void* ptr)
{
	::free(ptr);
}
