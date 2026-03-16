#pragma once
#include <string>
#include <unordered_map>
#include <functional>
#include "PValue.h"
#include "MetaFlagSet.h"

class PType;
class PField
{
public:
	PField(PType* type, const std::string& name, size_t offset);
	virtual ~PField();

	 const std::string& GetName();
	 const std::string& GetType();
	 uint64_t GetTypeHash();
	 void* GetValue(void* target);
	 void SetValue(void* target,void* value);
	 void SetFlag(long flag);
	 bool HasFlag(long flag);
private:
	PType* memberType;
	std::string memberName;
	size_t memberOffset;
	MetaFlagSet Flag;
};

