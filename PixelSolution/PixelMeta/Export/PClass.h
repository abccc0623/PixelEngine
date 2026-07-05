#pragma once
#include <unordered_map>
#include <vector>
#include <functional>
#include "PType.h"
#include "PValue.h"

using CreateFunction = void*(*)();
using DeleteFunction = void(*)();
using TypeCastFunction = void*(*)();

class PField;
class PMethod;
class PClass : public PType
{
public:
	PClass(uint64_t hash, std::string name, size_t size);
	virtual ~PClass();
	
	//등록
	void AddField(PField* field);
	void AddMethod(PMethod* method);

	//상속
	uint64_t GetParentHash();
	uint64_t GetChildHash();
	void SetParentHash(uint64_t hash);
	void SetChildHash(uint64_t hash);

	//변수
	int GetMemberCount();
	const std::string& GetMemberName(int index);
	const std::string& GetMemberType(int index);
	bool HasClassMemberFlag(int index, long flag);
	void* GetMemberValue(int index,void* target);
	void  SetMemberValue(int index,void* target,void* value);

	//함수
	int GetMethodCount();
	int GetPropertyCount(int index);
	bool HasClassMethodFlag(int index, long flag);
	const std::string& GetMethodName(int index);
	const std::string& GetMethodReturnType(int index);
	const std::string& GetMethodTooltip(int index);
	const std::string& GetMethodPropertyType(int index,int propertyIndex);
	const std::string& GetMethodPropertyName(int index, int propertyIndex);
	const std::string& GetMethodPropertyTooltip(int index, int propertyIndex);
	PValue CallMethod(int index,void* target, std::vector<void*> property);

	//생성 삭제
	void SetCreateFunction(CreateFunction func);
	void SetDeleteFunction(DeleteFunction func);
	void SetCastFunction(TypeCastFunction func);
	void* CallCreateFunction();
	void  CallDeleteFunction();

protected:
	uint64_t parentHash;
	uint64_t childHash;
	std::unordered_multimap<uint64_t, PField*> memberMap;
	std::unordered_multimap<uint64_t, PMethod*> methodMap;
	std::vector<PField*> memberList;
	std::vector<PMethod*> methodList;

	CreateFunction createFunc;
	DeleteFunction deleteFunc;
	TypeCastFunction castFunc;
};

