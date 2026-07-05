#pragma once
#include <unordered_map>
#include <vector>
#include <functional>
#include "PType.h"
#include "PValue.h"
class PMethod;
class PStatic : public  PType
{
public:
	PStatic(uint64_t hash, std::string name, size_t size);
	virtual ~PStatic();

	int GetMethodCount();
	int GetPropertyCount(int index);
	bool HasClassMethodFlag(int index, long flag);
	const std::string& GetMethodName(int index);
	const std::string& GetMethodReturnType(int index);
	const std::string& GetMethodTooltip(int index);
	const std::string& GetMethodPropertyType(int index, int propertyIndex);
	const std::string& GetMethodPropertyName(int index, int propertyIndex);
	const std::string& GetMethodPropertyTooltip(int index, int propertyIndex);
	PValue CallMethod(int index, void* target, std::vector<void*> property);
	void AddMethod(PMethod* method);
private:
	std::unordered_map<uint64_t, PMethod*> methodMap;
	std::vector<PMethod*> methodList;
};

