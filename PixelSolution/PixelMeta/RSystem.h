#pragma once
#include<string>
#include<unordered_map>
#include<vector>
#include <typeindex>
#include <typeinfo>

class PType;
class PObject;
class RSystem
{
public:
	RSystem();
	~RSystem();
	
	static uint64_t int_Type;
	static uint64_t float_Type;
	static uint64_t char_Type;
	static uint64_t double_Type;
	static uint64_t bool_Type;
	static uint64_t string_Type;

	PType* GetType(uint64_t hash);
	PType* GetTypeByString(std::string name);
	void Register(PType* type);
	void Release();

	std::unordered_map<uint64_t, PType*> MapByHash;
	std::vector<PType*> VectorByHash;
};

