#include "pch.h"
#include "PEnum.h"

PEnum::PEnum(uint64_t hash, std::string name, size_t size) : PType(hash, name, size)
{
	metaType = META_TYPE::ENUM;
}

PEnum::~PEnum()
{

}

void PEnum::AddEnum(const std::string& Key)
{
	enumList.push_back(Key);
}

int PEnum::GetEnumCount()
{
	return enumList.size();
}

const std::string& PEnum::GetEnum(int index)
{
	return enumList[index];
}
