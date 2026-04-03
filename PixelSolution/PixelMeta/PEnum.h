#pragma once
#include "PType.h"
#include <unordered_map>
#include <vector>
class PEnum : public PType
{
public:
	PEnum(uint64_t hash, std::string name, size_t size);
	virtual~PEnum();

	void AddEnum(const std::string& Key);
	int GetEnumCount();
	const std::string& GetEnum(int index);
private:
	std::vector<std::string> enumList;
};

