#include "pch.h"
#include "RSystem.h"
#include "PType.h"
#include "PixelMetaAPI.h"
#include "GlobalHashCode.h"
#include "PObject.h"

std::unordered_map<uint64_t, PType*> RSystem::MapByHash;
using string = std::string;

#define REGISTER_TYPE(type) \
auto Name##type = ExtractTypeName<type>(); \
auto Hash##type = HashUtil::ConstexprHash(Name##type.c_str()); \
auto Size##type = sizeof(type); \
MapByHash.insert({Hash##type,new PType(Hash##type,Name##type.c_str(),Size##type)});

RSystem::RSystem()
{
   REGISTER_TYPE(int32_t)
   REGISTER_TYPE(float)
   REGISTER_TYPE(char)
   REGISTER_TYPE(double)
   REGISTER_TYPE(bool)
   REGISTER_TYPE(string)
}

RSystem::~RSystem()
{
    for (auto& K : MapByHash)
    {
        delete K.second;
    }
    MapByHash.clear();
}

void RSystem::Release()
{
    delete this;
}

void RSystem::Register(PType* type)
{
    auto k = MapByHash.find(type->GetHash());
    if (k == MapByHash.end())
    {
        MapByHash.insert({ type->GetHash(), type });
    }
}

PType* RSystem::GetType(uint64_t hash)
{
    auto k = MapByHash.find(hash);
    if (k != MapByHash.end())
    {
        return MapByHash[hash];
    }
    return nullptr;
}

PObject* RSystem::CreateObject(void* target, std::string TypeName)
{
    uint64_t hash = HashUtil::ConstexprHash(TypeName.c_str());
    auto k = GetType(hash);
    PObject* obj = new PObject(target,k);
    return obj;
}
