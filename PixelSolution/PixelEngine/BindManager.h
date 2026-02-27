#pragma once
#include "EngineManager.h"
#include <unordered_map>
#include <functional>
#include <typeindex>
#include <string>
#include "sol.hpp"
#include "GlobalBind.h"


class GameObject;
class Module;
class BindManager : public EngineManager 
{
public:
	BindManager();
	~BindManager();

	void Initialize() override;
	void Update() override;
	void ReleaseShared() override;

	void AddModuleCall(std::string typeName, GameObject* obj);
	sol::object GetModuleCall_Lua(sol::this_state s, GameObject& obj, std::string name);

    template<typename T>
    static std::string ExportLuaAPIHeader()
    {
        //--- @class Transform
        //    Transform = {}
        std::string name = TypeName<std::decay_t<T>>::Get();
        std::string main = "";
        main += "--- @class ";
        main += name;
        main += "\n";
        main += name;
        main += " = {} \n\n";
        return main;
    }
    
    template <typename T, typename Ret, typename... Args>
	static inline std::string ExportLuaAPIFromFunc(std::string funcName, Ret(T::* func)(Args...), std::string paramNames)
	{
		return ExportLuaAPI<T, Ret, Args...>(funcName, paramNames);
	}

    template <typename T, typename Ret, typename... Args>
    static std::string ExportLuaAPI(std::string functionName, std::string paramNames)
    {
        std::string main = "";
        std::vector<std::string> MemberName = Split(paramNames, ',');
        std::vector<std::string> MemberType;

        ([&] { MemberType.push_back(TypeName<std::decay_t<Args>>::Get()); }(), ...);

        std::string member = "(";
        size_t typeCount = MemberType.size();
        for (size_t i = 0; i < typeCount; i++)
        {
            std::string name = (i < MemberName.size()) ? MemberName[i] : "arg" + std::to_string(i);
            main += "---@param " + name + " " + MemberType[i] + "\n";
            member += name + (i != typeCount - 1 ? ", " : "");
        }
        member += ") end";

        // 리턴 타입 추출
        std::string returnType = TypeName<std::decay_t<Ret>>::Get();
        main += "---@return " + returnType + "\n";

        main += "function " + TypeName<T>::Get() + ":" + functionName + member + "\n\n";
        return main;
    }
    static std::string apiDefinitions;
    static std::vector<std::string> bindClassName;
private:
	std::unordered_map<std::string, std::function<void(GameObject*)>>		    addModuleFunction;
	std::unordered_map<std::string, std::function<sol::object(sol::this_state, GameObject&, std::string)>>	getLuaModuleFunction;
    
    void CreateLuaAPI_File();
	static std::vector<std::string> Split(const std::string& s, char delimiter);
};