#pragma once
#include <string>
#include <vector>
#include "PixelMetaAPI.h"
template<typename MemberType ,typename ClassType>
class PFieldTemplate : public PField
{
public:
	PFieldTemplate(const std::string& fieldName, MemberType ClassType::* memberPtr)
	{
		type = typeid(MemberType).name();
		name = fieldName;
		offset = (size_t) & (((ClassType*)0)->*memberPtr);
	}
	virtual ~PFieldTemplate(){}
	void SetValue(void* instance, void* valuePtr) override
	{
		MemberType* targetAddr = reinterpret_cast<MemberType*>(reinterpret_cast<char*>(instance) + offset);
		if (valuePtr && targetAddr)
		{
			*targetAddr = *reinterpret_cast<MemberType*>(valuePtr);
		}
	}

	void* GetValuePtr(void* instance) override
	{
		return reinterpret_cast<char*>(instance) + offset;
	}
};
