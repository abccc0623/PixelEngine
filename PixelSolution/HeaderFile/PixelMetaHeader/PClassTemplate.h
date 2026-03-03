#pragma once
#include "PixelMetaAPI.h"
extern "C" PIXEL_META_DLL void* PixelInternalAlloc(size_t size);
template<typename T, typename ClassType> class PFieldTemplate;
template<typename ClassType> class PFieldString;

template<typename ClassType>
class PClassTemplate : public PClass
{
public:
    PClassTemplate()
    {
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
        auto k = members.find(name);
        if (k == members.end())
        {
            if constexpr (std::is_same_v<FieldType, std::string>)
            {
                members.insert({ name ,new PFieldString<ClassType>(name, memberPtr) });
            }
            else 
            {
                members.insert({ name ,new PFieldTemplate<FieldType, ClassType>(name, memberPtr) });
            }
        }
    }

    static PClassTemplate<ClassType>* Create()
    {
        void* rawMem = PixelInternalAlloc(sizeof(PClassTemplate<ClassType>));
        if (!rawMem) return nullptr;
        PClassTemplate<ClassType>* instance = new(rawMem) PClassTemplate<ClassType>();
        ReflectionClassRegister(instance);
        return instance;
    }
    ClassType* classValue;
};