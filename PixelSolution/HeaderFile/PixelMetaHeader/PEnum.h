#pragma once
#include <unordered_map>
#include <vector>
#include "PType.h"
class PEnum : public PType
{
public :
	PIXEL_META_DLL  PEnum();
	PIXEL_META_DLL  virtual ~PEnum();

	PIXEL_META_DLL void AddElement(const std::string& name,int index);
	PIXEL_META_DLL int GetElementByName(const std::string& name);
	PIXEL_META_DLL const std::string& GetElementByIndex(int index);
	PIXEL_META_DLL int GetElementMaxCount();
protected:
	std::vector<std::string> elements; // 순서 보장을 위해 vector
	std::unordered_map<std::string, int> nameToValue;
};

