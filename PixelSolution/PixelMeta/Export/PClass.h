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

    PIXEL_META_DLL PField* GetField(std::string name);
    
    std::string GetName();
    void Release();
protected:
    std::string type;
    std::string name;
    size_t offset;
    std::unordered_map<std::string, PField*> members;
};

