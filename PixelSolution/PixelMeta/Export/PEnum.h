#pragma once
#include <unordered_map>
#include <vector>
#include "PType.h"
class PEnum : public PType
{
public :
	PIXEL_META_DLL  PEnum();
	PIXEL_META_DLL  virtual ~PEnum();

	virtual PField* GetField(const std::string& name) = 0;
protected:
	std::unordered_map<uint64_t, PField*> memberMap;
	std::vector<PField*> memberList;
};

