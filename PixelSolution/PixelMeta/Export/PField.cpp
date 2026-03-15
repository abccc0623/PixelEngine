#include "pch.h"
#include "PField.h"
#include "PType.h"
#include "RSystem.h"
#include "PixelMetaAPI.h"
#include "GlobalHashCode.h"
#include "PValue.h"
PField::PField(PType* type, const std::string& name, size_t offset)
	:memberType(type), memberName(name), memberOffset(offset)
{


}
const std::string& PField::GetName()
{
	return memberName;
}
const std::string& PField::GetType()
{
	return memberType->GetName();
}
uint64_t PField::GetTypeHash()
{
	return memberType->GetHash();
}

PField::~PField()
{

}
void PField::SetValue(void* target, void* value)
{
	void* member = reinterpret_cast<char*>(target) + memberOffset;
	if (memberType->GetHash() == RSystem::string_Type)
	{
		auto k =  *static_cast<std::string*>(member);
	}
	else
	{
		std::memcpy(member, value,memberType->GetSize());
	}
}

void* PField::GetValue(void* target)
{
	return (reinterpret_cast<char*>(target) + memberOffset);
}



