#pragma once
#include <unordered_map>
#include <vector>
#include "PType.h"
class PField;
class PMethod;
class PClass : public PType
{
public:
	PIXEL_META_DLL PClass();
	PIXEL_META_DLL virtual ~PClass();

	virtual PField* GetField(const std::string& name) = 0;
	virtual PMethod* GetMethod(const std::string& name) = 0;

	PIXEL_META_DLL int GetFieldMaxCount();
	PIXEL_META_DLL int GetMethodMaxCount();
	PIXEL_META_DLL const std::string& TypeName();
	PIXEL_META_DLL const std::string& ParentName();
	PIXEL_META_DLL const std::string& ChildName();

	PIXEL_META_DLL const std::string& GetFieldName(int index);
	PIXEL_META_DLL const std::string& GetFieldType(int index);
protected:
	PClass* parent;
	PClass* child;
	std::unordered_map<uint64_t, PField*> memberMap;
	std::unordered_map<uint64_t, PMethod*> methodMap;
	std::vector<PField*> memberList;
	std::vector<PMethod*> methodList;
};

