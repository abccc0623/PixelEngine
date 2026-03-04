#include "pch.h"
#include "PField.h"

PField::PField(const std::string& typeName, const std::string& tieldname, size_t offset) :
	type(typeName), name(tieldname), offset(offset)
{

}

PField::~PField()
{

}

void PField::SetValue(void* instance, void* valuePtr)
{

}

void* PField::GetValuePtr(void* instance)
{
	return reinterpret_cast<char*>(instance) + offset;
}

std::string PField::GetFieldName()
{
	return name;
}

std::string PField::GetFieldType()
{
	return type;
}

