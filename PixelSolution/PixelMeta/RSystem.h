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
	static RSystem& GetInstance()
	{
		static RSystem instance;
		return instance;
	}
	RSystem(const RSystem&) = delete;
	RSystem& operator=(const RSystem&) = delete;
	RSystem(RSystem&&) = delete;
	RSystem& operator=(RSystem&&) = delete;
	
	static uint64_t int_Type;
	static uint64_t float_Type;
	static uint64_t char_Type;
	static uint64_t double_Type;
	static uint64_t bool_Type;
	static uint64_t string_Type;

	int GetTypeAllCount();
	PType* GetTypeByIndex(int index);


	PType* GetType(uint64_t hash);
	PType* GetTypeByString(std::string name);
	void Register(PType* type);
	void Release();

	std::unordered_map<uint64_t, PType*> MapByHash;
	std::vector<PType*> VectorByHash;
private:
	RSystem();
	~RSystem();
};

