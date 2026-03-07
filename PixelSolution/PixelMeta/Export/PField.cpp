#include "pch.h"
#include "PField.h"
#include "PType.h"
#include "PixelMetaAPI.h"
PField::PField(PType* type, const std::string& name, size_t offset)
	:memberType(type), memberName(name), memberOffset(offset)
{


}
std::string PField::GetName()
{
	return memberName;
}
std::string PField::GetType()
{
	return memberType->GetName();
}
PValue PField::GetValuePtr(void* instance)
{
	return valueGetter(instance, memberOffset);
}
void PField::SetValue(void* instance, PValue value)
{
	void* target = reinterpret_cast<char*>(instance) + memberOffset;
	if (memberType == GetTypeByKeyword<std::string>())
	{
		*static_cast<std::string*>(target) = value.AsChar();
	}
	else
	{
		std::memcpy(target, &value.Data,memberType->GetSize());
	}
}
void* PField::operator new(size_t size)
{
	return ::malloc(size);
}

void PField::operator delete(void* ptr)
{
	::free(ptr);
}

PField::~PField()
{

}

//void PField::SetValue(void* instance, void* valuePtr)
//{
//
//}
//
//void* PField::GetValuePtr(void* instance)
//{
//	return reinterpret_cast<char*>(instance) + offset;
//}
//
//std::string PField::GetFieldName()
//{
//	return name;
//}
//
//std::string PField::GetFieldType()
//{
//	return type;
//}


