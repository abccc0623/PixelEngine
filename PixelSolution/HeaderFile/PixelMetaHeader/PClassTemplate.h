#pragma once
#include <string>
#include <unordered_map>
#include <functional>
#include <type_traits>
#include <vector>
#include <cstddef>
#include <iostream>
#include <utility>
#include "PixelMetaDLL.h"
#include "GlobalHashCode.h"
#include "PType.h"
#include "PClass.h"
#include "PField.h"
#include "PMethod.h"
#include <iostream>


class PMethod;
template<typename ThisType,typename ParentType = void>
class PClassTemplate : public PClass
{
public:
    PClassTemplate() : PClass()
    {
        if constexpr (!std::is_same_v<ParentType, void>)
        {
            PType* type = GetTypeByKeyword<ParentType>();
            if (type != nullptr)  parent = static_cast<PClass*>(type);
        }
        typeName = ExtractTypeName<ThisType>();
        typeHash = HashUtil::ConstexprHash(typeName.c_str());
        typeSize = sizeof(ThisType);
    }
    virtual ~PClassTemplate(){}

    PField* GetField(const std::string& name) override
    {
        uint64_t nameHash = HashUtil::ConstexprHash(name.c_str());
        auto find = memberMap.find(nameHash);
        if (find != memberMap.end())
        {
            return find->second;
        }
        else if (parent == nullptr)
        {
            return nullptr;
        }
        else
        {
            return parent->GetField(name);

        }
    }

    PMethod* GetMethod(const std::string& name) override
    {
        uint64_t nameHash = HashUtil::ConstexprHash(name.c_str());
        auto find = methodMap.find(nameHash);
        if (child != nullptr)
        {
            return nullptr;
        }
        else if (find != methodMap.end())
        {
            return find->second;
        }
        else
        {
            return nullptr;
        }
    }


    template<typename FieldType>
    void AddField(const std::string& name, FieldType ThisType::*memberPtr)
    {
        uint64_t nameHash   = HashUtil::ConstexprHash(name.c_str());
        size_t offset       = reinterpret_cast<size_t>(&(((ThisType*)0)->*memberPtr));
        PType* type         = GetTypeByKeyword<FieldType>();
        ///해당 타입이 등록이 안되어 있을때
        if (type == nullptr){return;}

        ///같은 맴버를 또 넣었을 때 체크
        auto find =memberMap.find(nameHash);
        if (find != memberMap.end()){return;}

        ///정상적으로 등록
        PField* field = new PField(type, name, offset);
        field->valueGetter = [](void* instance,size_t _offset)-> PValue
            {
                void* target = reinterpret_cast<char*>(instance) + _offset;
                if constexpr (std::is_same_v<FieldType, int32_t> ||
                    std::is_same_v<FieldType, float> ||
                    std::is_same_v<FieldType, char> ||
                    std::is_same_v<FieldType, double> ||
                    std::is_same_v<FieldType, bool>)
                {
                    return PValue(static_cast<FieldType*>(target));
                }
                else
                {
                    return PValue(static_cast<void*>(target));
                }
            };
        memberMap.insert({ nameHash, field});
        memberList.push_back(field);
    }

    template<typename ClassType, typename ReturnType, typename... Args>
    void AddMethod(std::string name, ReturnType(ClassType::* Func)(Args...))
    {
        ([&] {
            // 여기서 Args는 루프를 돌 때마다 각 타입으로 치환됨
                PType* typeInfo = GetTypeByKeyword<Args>();
                std::cout << typeInfo->GetName() << std::endl;
            }(), ...);

        uint64_t nameHash = HashUtil::ConstexprHash(name.c_str());
        auto m = new PMethod(name);
        if constexpr (std::is_void_v<ReturnType>)
        {
            m->NoReturnFunction = [Func](void* target, std::vector<void*>& argList) ->  void*
                {
                    auto k = std::make_index_sequence<sizeof...(Args)>{};
                    ClassType* C = static_cast<ClassType*>(target);
                    PMethod::CallHelper(C, Func, argList, k);
                    return nullptr;
                };
        }
        else
        {
            m->NoReturnFunction = [Func](void* target, std::vector<void*>& argList) -> void*
                {
                    auto k = std::make_index_sequence<sizeof...(Args)>{};
                    ClassType* C = static_cast<ClassType*>(target);
                    PValue t = PMethod::ReturnCallHelper(C, Func, argList, k);
                    return nullptr;
                };
        }
        methodMap.insert({ nameHash, m });
        methodList.push_back(m);
    }
};

