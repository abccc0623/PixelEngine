#pragma once
#include <unordered_map>
#include <vector>
#include <functional>
#include "PType.h"
#include "PValue.h"
class PMethod;
class PNamespace : public  PType
{
public:
	PNamespace(uint64_t hash, std::string name, size_t size);
	virtual ~PNamespace();

	int GetMethodCount();
	int GetPropertyCount(int index);
	bool HasClassMethodFlag(int index, long flag);
	const std::string& GetMethodName(int index);
	const std::string& GetMethodReturnType(int index);
	const std::string& GetMethodPropertyType(int index, int propertyIndex);
	PValue CallMethod(int index, void* target, std::vector<void*> property);
	void AddMethod(PMethod* method);
private:
	std::unordered_map<uint64_t, PMethod*> methodMap;
	std::vector<PMethod*> methodList;
};

