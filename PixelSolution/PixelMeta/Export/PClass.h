#pragma once
#include <string>
#include <unordered_map>
#include "PixelMetaDLL.h"

class PField;
class PClass
{
public:
    PIXEL_META_DLL PClass();
    PIXEL_META_DLL virtual ~PClass();

    PIXEL_META_DLL PField* GetFieldByName(std::string name);
    
    std::string GetName();
    void Release();
protected:
    PClass* parent;
    uint64_t parentHash = 0;

    std::string type;
    std::string name;
    size_t offset;
    std::unordered_map<uint64_t, PField*> members;
};

