#include "pch.h"
#include "PType.h"
#include "PField.h"
#include "PMethod.h"

PType::PType(uint64_t Hash, std::string name, size_t size):
	typeHash(Hash), typeName(name), typeSize(size)
{
	metaType = META_TYPE::PRIMITIVE;
}
PType::PType():
	typeHash(0), typeName(""), typeSize(0)
{

}
PType::~PType()
{
}
const std::string& PType::GetName()
{
	if (this == GetTypeByKeyword<std::string>())
	{
		typeName = "string";
	}
	return typeName;
}
const std::string& PType::GetMetaType()
{
	switch (metaType)
	{
	case META_TYPE::PRIMITIVE:
		return "PRIMITIVE";
	case META_TYPE::CLASS:
		return "CLASS";
	case META_TYPE::ENUM:
		return "ENUM";
	}
}
uint64_t PType::GetHash()
{
	return typeHash;
}
uint64_t PType::GetSize()
{
	return typeSize;
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
