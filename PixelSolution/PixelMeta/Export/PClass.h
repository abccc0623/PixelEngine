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
#include "PField.h"
#include "PMethod.h"

template<typename T>
T& GetArg(std::vector<void*>& args, size_t index)
{
    return *static_cast<T*>(args[index]);
}

using MethodPtr = std::function<PValue(void*, std::vector<void*>&)>;
class PMethod;
template<typename ThisType,typename ParentType = void>
class PClass : public PType
{
public:
    PClass() : PType()
    {
        if constexpr (!std::is_same_v<ParentType, void>)
        {
            PType* type = GetTypeByKeyword<ParentType>();
            if (type != nullptr)  parent = type;
        }
        typeName = ExtractTypeName<ThisType>();
        typeHash = HashUtil::ConstexprHash(typeName.c_str());
        typeSize = sizeof(ThisType);
    }
    virtual ~PClass()
    {
        for (auto& K : memberList)
        {
            delete K.second;
        }
        for (auto& K : methodList)
        {
            delete K.second;
        }
        memberList.clear();
    }

    PField* GetField(const std::string& name) override
    {
        uint64_t nameHash = HashUtil::ConstexprHash(name.c_str());
        auto find = memberList.find(nameHash);
        if (find != memberList.end())
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
        auto find = methodList.find(nameHash);
        if (child != nullptr)
        {
            return nullptr;
        }
        else if (find != methodList.end())
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
        auto find =memberList.find(nameHash);
        if (find != memberList.end()){return;}

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
        memberList.insert({ nameHash, field});
    }

    template<typename RetrunType, typename... Args>
    void AddMethod(std::string name, RetrunType(ThisType::*Func)(Args...))
    {
        std::vector<PType*> list;
        list.reserve(sizeof...(Args));
        (list.push_back(GetTypeByKeyword<Args>()),...);
        PType* returnType = GetTypeByKeyword<RetrunType>();
        uint64_t nameHash = HashUtil::ConstexprHash(name.c_str());
        auto m = new PMethod(name, returnType,list);

        m->templateFunction = [Func](void* retAddr, std::vector<void*>& args)-> PValue
            {
                size_t arraySize = sizeof...(Args);
                if (args.size() != arraySize) return nullptr;

                //실제함수를 실행할 타겟
                auto callTarget = static_cast<ThisType*>(retAddr);

                //args를 args[0],args[1],args[2] 로 넘겨줌
                auto k = std::make_index_sequence<sizeof...(Args)>{};
                ThisType* obj = static_cast<ThisType*>(retAddr);
                //실제 float,int 형으로 실행시키기 위해 헬퍼 함수를 다시 실행
                //PValue num = PMethod::CallHelper<Args...>(obj, Func, retAddr, args, k, TypeList<Args...>{});
                return PValue();
            };
        methodList.insert({ nameHash,m });
    }
private:
    PType* parent;
    PType* child;
    std::unordered_map<uint64_t, PField*> memberList;
    std::unordered_map<uint64_t, PMethod*> methodList;
};

