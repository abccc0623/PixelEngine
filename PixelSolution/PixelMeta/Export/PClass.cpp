#include "pch.h"
#include "PClass.h"

PClass::PClass():type(),name(),offset(),members()
{
    
}

PClass::~PClass()
{

}

PField* PClass::GetField(std::string name)
{
    auto k = members.find(name);
    if (k != members.end())
    {
        return k->second;
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
