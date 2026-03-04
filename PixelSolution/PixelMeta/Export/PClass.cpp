#include "pch.h"
#include "PClass.h"
#include "PixelMetaAPI.h"
PClass::PClass():type(),name(),offset(),members()
{
    
}

PClass::~PClass()
{

}

PField* PClass::GetFieldByName(std::string name)
{
    uint64_t nameHash = HashUtil::ConstexprHash(name.c_str());
    auto k = members.find(nameHash);
    if (k != members.end())
    {
        return k->second;
    }
    else
    {
        //부모가 있다면 부모에서 찾음
        if (parentHash != 0)
        {
            PClass* parent = FindClassByHash(parentHash);
            auto k = parent->members.find(nameHash);
            if (k != parent->members.end())
            {
                return k->second;
            }
        }
    }
    return nullptr;
}

std::string PClass::GetName()
{
	return name;
}

void PClass::Release()
{
    for (auto& k : members)
    {
        delete k.second;
    }
    members.clear();
}
