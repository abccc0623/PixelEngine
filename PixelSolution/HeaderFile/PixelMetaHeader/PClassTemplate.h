#pragma once
#include "PixelMetaAPI.h"
#include "GlobalMetaFunc.h"
#include <type_traits>
extern "C" PIXEL_META_DLL void* PixelInternalAlloc(size_t size);
extern "C" PIXEL_META_DLL PField* CreateMetaType(uint64_t hash, const std::string& fieldName, size_t offset, size_t size);
extern "C" PIXEL_META_DLL void ReflectionClassRegister(uint64_t hash, PClass* targetClass);
extern "C" PIXEL_META_DLL PClass* FindClassByName(std::string ClassName);

template<typename ClassType, typename ParentClass = void>
class PClassTemplate : public PClass
{
public:
    PClassTemplate()
    {
        //상속일때 부모의 해쉬를 저장
        if constexpr (!std::is_same_v<ParentClass, void>)
        {
            constexpr uint64_t typeId = TypeHash<ParentClass>::Value();
            parentHash = typeId;
        }

        type = "class";
        name = typeid(ClassType).name();
        const std::string classPrefix = "class ";
        size_t pos = name.find(classPrefix);
        if (pos != std::string::npos)
        {
            name.erase(pos, classPrefix.length());
        }

        offset = reinterpret_cast<char*>(&classValue) - reinterpret_cast<char*>(this);
    }
    virtual ~PClassTemplate() = default;


    template<typename FieldType>
    void AddField(const std::string& name, FieldType ClassType::* memberPtr)
    {
        uint64_t nameHash = HashUtil::ConstexprHash(name.c_str());
        auto k = members.find(nameHash);
        if (k == members.end())
        {
            if constexpr (std::is_pointer_v<FieldType>)
            {
                //포인터타입 나중에...
                using ElementType = std::remove_pointer_t<FieldType>;

                constexpr uint64_t elementHash = TypeHash<ElementType>::Value();
                size_t offset = (size_t) & (((ClassType*)0)->*memberPtr);

            }
            else
            {
                //일반 변수 타입
                std::string memberFieldname = name;
                size_t memberOffset = (size_t) & (((ClassType*)0)->*memberPtr);
                size_t size = sizeof(FieldType);

                constexpr uint64_t typeId = TypeHash<FieldType>::Value();
                members.insert({ nameHash ,CreateMetaType(typeId,memberFieldname,memberOffset,size) });
            }
        }
    }


    static PClassTemplate<ClassType,ParentClass>* Create()
    {
        void* rawMem = PixelInternalAlloc(sizeof(PClassTemplate<ClassType, ParentClass>));
        if (!rawMem) return nullptr;
        PClassTemplate<ClassType, ParentClass>* instance = new(rawMem) PClassTemplate<ClassType, ParentClass>();
        constexpr uint64_t typeId = TypeHash<ClassType>::Value();
        ReflectionClassRegister(typeId,instance);
        return instance;
    }
    ClassType* classValue;
};