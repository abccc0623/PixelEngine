#include "pch.h"
#include "RSystem.h"
#include "PType.h"
#include "PixelMetaAPI.h"
#include "GlobalHashCode.h"
#include "PObject.h"

using string = std::string;

#define REGISTER_TYPE(type) \
auto Name##type = ExtractTypeName<type>(); \
auto Hash##type = HashUtil::ConstexprHash(Name##type.c_str()); \
auto Size##type = sizeof(type); \
auto Type##type = new PType(Hash##type,Name##type.c_str(),Size##type); \
MapByHash.insert({Hash##type,Type##type}); \
VectorByHash.push_back(Type##type);


uint64_t RSystem::int_Type;
uint64_t RSystem::float_Type;
uint64_t RSystem::char_Type;
uint64_t RSystem::double_Type;
uint64_t RSystem::bool_Type;
uint64_t RSystem::string_Type;
RSystem::RSystem()
{
   REGISTER_TYPE(int32_t)
   REGISTER_TYPE(float)
   REGISTER_TYPE(char)
   REGISTER_TYPE(double)
   REGISTER_TYPE(bool)
   REGISTER_TYPE(string)

   int_Type = HashUtil::ConstexprHash("int");
   float_Type = HashUtil::ConstexprHash("float");
   char_Type = HashUtil::ConstexprHash("char");
   double_Type = HashUtil::ConstexprHash("double");
   bool_Type = HashUtil::ConstexprHash("bool");
   string_Type = HashUtil::ConstexprHash("string");
}

RSystem::~RSystem()
{
    for (auto& K : MapByHash)
    {
        delete K.second;
    }
    MapByHash.clear();
}
PType* RSystem::GetTypeByString(std::string name)
{
    if (name == "string")
    {
        name = ExtractTypeName<string>();
    }
    uint64_t hash = HashUtil::ConstexprHash(name.c_str());
    auto k = MapByHash.find(hash);
    if (k != MapByHash.end())
    {
        return MapByHash[hash];
    }
    return nullptr;
}

void RSystem::Register(PType* type)
{
    auto k = MapByHash.find(type->GetHash());
    if (k == MapByHash.end())
    {
        MapByHash.insert({ type->GetHash(), type });
        VectorByHash.push_back(type);
    }
}

void RSystem::Release()
{
    for (auto& K : MapByHash)
    {
        delete K.second;
    }
    VectorByHash.clear();
    MapByHash.clear();
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
//
//PObject* RSystem::CreateObject(void* target, std::string TypeName)
//{
//    uint64_t hash = HashUtil::ConstexprHash(TypeName.c_str());
//    auto k = GetType(hash);
//    PObject* obj = new PObject(target,k);
//    return obj;
//}


