#pragma once
#include<string>
#include<unordered_map>
#include <typeindex>
#include <typeinfo>

class PType;
class PObject;
class RSystem
{
public:
	RSystem();
	~RSystem();

	void Release();
	void Register(PType* type);
	PType* GetType(uint64_t hash);
	PObject* CreateObject(void* target, std::string TypeName);
	static std::unordered_map<uint64_t, PType*> MapByHash;

};

