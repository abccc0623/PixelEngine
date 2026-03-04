#pragma once
#include "PField.h"
#include "PixelMetaDLL.h"

template<typename T>
class PFieldData :public PField
{
public:
    PFieldData(const std::string& type, const std::string& name, size_t offset):PField(type, name, offset){}
    virtual ~PFieldData() {};

    virtual void SetValue(void* instance, void* valuePtr)
    {
        T* targetAddr = reinterpret_cast<T*>(reinterpret_cast<char*>(instance) + offset);
        if (valuePtr && targetAddr)
        {
            *targetAddr = *reinterpret_cast<T*>(valuePtr);
        }
    };
    virtual void* GetValuePtr(void* instance) 
    {
        return reinterpret_cast<T*>(reinterpret_cast<char*>(instance) + this->offset);;
    }
};

template<>
class PFieldData<std::string> : public PField
{
public:
    PFieldData(const std::string& type, const std::string& name, size_t offset) :PField(type, name, offset) {}
    virtual ~PFieldData() {};

    virtual void SetValue(void* instance, void* valuePtr)
    {
        if (!instance || !valuePtr) return;
        std::string* target = reinterpret_cast<std::string*>(reinterpret_cast<char*>(instance) + this->offset);
        std::string* source = reinterpret_cast<std::string*>(valuePtr);
        *target = *source;
    };
    virtual void* GetValuePtr(void* instance)
    {
        return reinterpret_cast<std::string*>(reinterpret_cast<char*>(instance) + this->offset);;
    }
};

class PFieldObject :public PField
{
public:
    PFieldObject(const std::string& type, const std::string& name, size_t offset, size_t size) :PField(type, name, offset) { typeSize = size; }
    virtual ~PFieldObject() {};

    virtual void SetValue(void* instance, void* valuePtr)
    {
        void* targetAddr = reinterpret_cast<char*>(instance) + offset;
        if (valuePtr && targetAddr) {
            std::memcpy(targetAddr, valuePtr, typeSize);
        }
    };
    virtual void* GetValuePtr(void* instance)
    {
        return reinterpret_cast<char*>(instance) + offset;
    }
    size_t typeSize;
};
