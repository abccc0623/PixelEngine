#include "pch.h"
#include "PObject.h"
#include "PType.h"
#include "PClass.h"
#include "PField.h"

PObject::PObject(void* target,PType* type):
	target(target)
{
	if(type)

	Classtype = static_cast<PClass*>(type);
}

PObject::~PObject()
{

}

PValue PObject::Get(std::string memberName)
{
	PField* field = Classtype->GetField(memberName);
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
	PField* field = Classtype->GetField(memberName);
	field->SetValue(target, value);
}

int PObject::GetFieldMaxCount()
{
	return Classtype->GetFieldMaxCount();
}

int PObject::GetMethodMaxCount()
{
	return Classtype->GetMethodMaxCount();
}

const std::string& PObject::TypeName()
{
	return Classtype->TypeName();
}

const std::string& PObject::ParentName()
{
	return Classtype->ParentName();
}

const std::string& PObject::ChildName()
{
	return Classtype->ChildName();
}

const std::string& PObject::GetFieldName(int index)
{
	return Classtype->GetFieldName(index);
}

const std::string& PObject::GetFieldType(int index)
{
	return Classtype->GetFieldType(index);
}
