#include "pch.h"
#include "PObject.h"
#include "PType.h"
#include "PField.h"
PObject::PObject(void* target,PType* type):
	target(target), type(type)
{

}

PObject::~PObject()
{

}

const char* PObject::GetTypeName(PObject* obj)
{
	return nullptr;
}

const char* PObject::GetFieldName(int index)
{
	return nullptr;
}

int PObject::GetFieldCount()
{
	return 0;
}

PValue PObject::Get(std::string memberName)
{
	PField* field = type->GetField(memberName);
	if (field != nullptr)
	{
		return field->GetValuePtr(target);
	}
	else
	{
		return PValue();
	}
}

void PObject::Set(std::string memberName,PValue value)
{
	PField* field = type->GetField(memberName);
	field->SetValue(target, value);
}
