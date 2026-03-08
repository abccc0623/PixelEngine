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

template<typename T>
T& GetArg(std::vector<void*>& args, size_t index)
{
    return *static_cast<T*>(args[index]);
}

using MethodPtr = std::function<PValue(void*, std::vector<void*>&)>;
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

    template <typename F>
    void AddMethod(std::string name, F&& lambda)
    {
        //using Traits = FunctionTraits<decltype(&std::remove_reference_t<F>::operator())>;
        //using ReturnType = typename Traits::ReturnType;
        //
        //if constexpr (std::is_void_v<ReturnType>)
        //{
        //    PType* returnType = GetTypeByKeyword<ReturnType>();
        //    uint64_t nameHash = HashUtil::ConstexprHash(name.c_str());
        //    //auto m = new PMethod(name, returnType, list);
        //}


        //std::vector<PType*> list;
        //list.reserve(sizeof...(_Args));
        //(list.push_back(GetTypeByKeyword<_Args>()), ...);
        //
        //PType* returnType = GetTypeByKeyword<_Ret>();
        //uint64_t nameHash = HashUtil::ConstexprHash(name.c_str());
        //auto m = new PMethod(name, returnType, list);
        //
        //if constexpr (std::is_void_v<_Ret>)
        //{
        //    m->templateFunction = nullptr;
        //}
        //
        //// 1. Func는 '값'이므로 반드시 캡처해야 합니다.
        //// 2. ReturnType과 Args는 '타입'이므로 람다 내부에서 그냥 사용 가능합니다.
        //m->templateFunction = [Func](void* retAddr, std::vector<void*>& args) -> PValue
        //    {
        //        size_t arraySize = sizeof...(_Args);
        //        if (args.size() != arraySize) return PValue(); // nullptr 대신 PValue()
        //
        //        // Class는 이미 템플릿 인자로 들어와 있으므로 ThisType 대신 Class를 직접 써보세요.
        //        _Class* obj = static_cast<_Class*>(retAddr);
        //        auto k = std::make_index_sequence<sizeof...(_Args)>{};
        //
        //        // 여기서 ReturnType은 템플릿 인자로 고정된 상태입니다.
        //        if constexpr (std::is_void_v<_Ret>)
        //        {
        //            PMethod::CallHelper(obj, Func, args, k);
        //            return PValue();
        //        }
        //        else
        //        {
        //            // PVector3::Dot의 경우 ReturnType은 float이므로 반드시 여기를 탑니다.
        //            return PValue(PMethod::CallHelper(obj, Func, args, k));
        //        }
        //    };
        //
        //methodMap.insert({ nameHash, m });
        //methodList.push_back(m);
    }
};

