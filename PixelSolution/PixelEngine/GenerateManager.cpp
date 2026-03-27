#include "pch.h"
#include "GenerateManager.h"
#include "PixelMetaAPI.h"
#include "LuaGenerateTemplate.h"
#include "PixelMeta.h"
#include "LuaBind.h"
#include "LSPBind.h"
using json = nlohmann::json;
void GenerateManager::Initialize()
{
    luaBind = new LuaBind();
    lspBind = new LSPBind();
}

void GenerateManager::Update()
{

}

void GenerateManager::Release()
{
    delete luaBind;
    delete lspBind;
}

void GenerateManager::Clear()
{

}

void GenerateManager::CreateBindCode()
{
    types.clear();
    int allCount = GetTypeAllCount();
    for (int typeIndex = 0; typeIndex < allCount; typeIndex++)
    {
        PType* targetType       = GetTypeByIndex(typeIndex);
        auto metaType           = GetTypeCategory(targetType);
        std::string TypeName    = GetTypeName(targetType);


        PixelClassMeta PixelClass;
        if (metaType == META_TYPE::CLASS)
        {
            PClass* PClass  = GetClass(TypeName);
            PixelClass.thisName = TypeName;
            PixelClass.metaType = metaType;
            int memberCount = GetClassMemberCount(PClass);
            int methodCount = GetClassMethodCount(PClass);
            PixelClass.parentHash  = GetClassParentHash(PClass);
            for (int memberIndex = 0; memberIndex < memberCount; memberIndex++)
            {
                PixelMemberMeta PixelMember;
                PixelMember.name = GetClassMemberName(PClass, memberIndex);
                PixelMember.type = GetClassMemberType(PClass, memberIndex);
                PixelClass.members.push_back(PixelMember);
            }
            for (int methodIndex = 0; methodIndex < methodCount; methodIndex++)
            {
                if (HasClassMethodFlag(PClass, methodIndex, MetaFlag::LUABIND) == false) continue;
                PixelMethodMeta PixelMethod;
                PixelMethod.name            = GetClassMethodName(PClass, methodIndex);
                PixelMethod.returnType      = GetClassMethodReturnType(PClass, methodIndex);
                int methodPropertyCount     = GetClassMethodPropertyCount(PClass, methodIndex);
                for (int propertyIndex = 0; propertyIndex < methodPropertyCount; propertyIndex++)
                {
                    PixelMethod.propertys.push_back(GetClassMethodGetPropertyType(PClass, methodIndex, propertyIndex));
                }
                PixelClass.methods.push_back(PixelMethod);
            }
             types.push_back(PixelClass);
        }
        else if (metaType == META_TYPE::NAMESPACE)
        {
            PNamespace* PNamespace = GetNamespace(TypeName);
            PixelClass.metaType = metaType;
            PixelClass.thisName = TypeName;
            int methodCount     = GetNamespaceMethodCount(PNamespace);
            for (int methodIndex = 0; methodIndex < methodCount; methodIndex++)
            {
                if (HasNamespaceMethodFlag(PNamespace, methodIndex, MetaFlag::LUABIND) == false) continue;
                PixelMethodMeta PixelMethod;
                PixelMethod.name = GetNamespaceMethodName(PNamespace, methodIndex);
                PixelMethod.returnType = GetNamespaceMethodReturnType(PNamespace, methodIndex);
                int methodPropertyCount = GetNamespaceMethodPropertyCount(PNamespace, methodIndex);
                for (int propertyIndex = 0; propertyIndex < methodPropertyCount; propertyIndex++)
                {
                    PixelMethod.propertys.push_back(GetNamespaceMethodGetPropertyType(PNamespace, methodIndex, propertyIndex));
                }
                PixelClass.methods.push_back(PixelMethod);
            }
            types.push_back(PixelClass);
        }
        else
        {
            continue;
        }
    }
}

void GenerateManager::LuaGenerate(const char* outPath)
{
    CreateBindCode();
    luaBind->Generate(outPath, types);
}

void GenerateManager::LSPGenerate(const char* outPath)
{
    CreateBindCode();
    lspBind->Generate(outPath, types);
}

void GenerateManager::JsonGenerate(const char* outPath)
{

}







