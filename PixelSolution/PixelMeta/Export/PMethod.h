#pragma once
#include <string>
#include <unordered_map>
#include <functional>
#include "PixelMetaDLL.h"
#include "PValue.h"
#include "MetaFlagSet.h"

class PField;
class PMethod
{
public:
	PMethod(std::string name);
	~PMethod();
	const std::string& GetName();
	const std::string& GetReturnType();
	const std::string& GetPropertyType(int index);
	int GetPropertyCount();

	PValue Call(void* target, std::vector<void*> property);

	void SetFlag(long flag);
	bool HasFlag(long flag);
	uint64_t GetTypeHash();
	void SetInfo(std::string retrunType, std::string classType, std::vector<std::string> memberType, std::function<PValue (void*, std::vector<void*>&)> func);
protected:
	std::string retrunType;
	std::string classType;
	std::vector<std::string> propertyType;
	uint64_t typeHash;
	std::string methodName;
	std::function<PValue (void*, std::vector<void*>&)> invoker;
	MetaFlagSet Flag;
};


