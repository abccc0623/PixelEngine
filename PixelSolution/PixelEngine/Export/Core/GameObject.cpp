#include "pch.h"
#include "GameObject.h"
#include "Core/Module.h"
#include "PixelEngine.h"
#include "FunctionManager.h"
#include "Module/Transform.h"
#include "SerializeHelper.h"
#include "Type/GlobalEnum.h"
#include "SPointer.h"
#include "GlobalBind.h"

extern PixelEngine* Engine;
FunctionManager* GameObject::functionManager = nullptr;
GameObject::GameObject()
{
	ModuleMap = std::unordered_map<MODULE_TYPE, SPointer<Module>>();
	hashCode = std::hash<GameObject*>{}(this);
	if (functionManager == nullptr)
	{
		functionManager = Engine->GetFactory<FunctionManager>();
	}
}
GameObject::~GameObject()
{
	ModuleMap.clear();
}

void GameObject::AddModule(MODULE_TYPE type)
{
	const auto info = ModuleTypeManager::GetTypeInfo(type);
	const auto s = SPointer<Module>(info.CreateModule());
	if (info.functionCheck[(int)MODULE_FUNC::AWAKE] == true) functionManager->AddOneTimeFunction(s, AWAKE_FUNCTION);
	if (info.functionCheck[(int)MODULE_FUNC::START] == true) functionManager->AddOneTimeFunction(s, START_FUNCTION);
	if (info.functionCheck[(int)MODULE_FUNC::UPDATE] == true) functionManager->AddTickFunction(s, UPDATE_FUNCTION);
	if (info.functionCheck[(int)MODULE_FUNC::MATRIX] == true) functionManager->AddTickFunction(s, MATRIX_UPDATE_FUNCTION);
	if (info.functionCheck[(int)MODULE_FUNC::PHYSICS] == true) functionManager->AddTickFunction(s, PHYSICS_UPDATE_FUNCTION);
	if (info.functionCheck[(int)MODULE_FUNC::LAST] == true) functionManager->AddTickFunction(s, LAST_UPDATE_FUNCTION);
	s->targetObject = this;
	s->type = info.type;
	ModuleMap.insert({ type,s });
	auto tr = GetModule(MODULE_TYPE::Transform);
	if (tr != nullptr)
	{
		s->transform = static_cast<Transform*>(GetModule(MODULE_TYPE::Transform));
	}
	else
	{
		std::cout<< "[AddModule] : Not Find Transform" << std::endl;
	}
}

bool GameObject::HasModule(MODULE_TYPE type)
{
	auto k = ModuleMap.find(type);
	return (k == ModuleMap.end()) ? false : true;
}

Module* GameObject::GetModule(MODULE_TYPE type)
{
	if (HasModule(type))
	{
		return ModuleMap[type].GetPtr();
	}
	else
	{
		return nullptr;
	}
}

unsigned long GameObject::GetHash()
{
	return hashCode;
}

void GameObject::Destroy()
{
	
}

#ifdef PIXEL_EDITOR
std::string GameObject::Save(int tab)
{
	//std::string content = BeginBlock(tab); // 개별 오브젝트 { 시작
	//content += AddEntry(tab + 1, "Name", name);
	//content += AddEntry(tab + 1, "HashCode", (float)hashCode);
	//
	//content += BeginBlock(tab + 1, "Modules"); // Modules = { 시작
	//for (auto& pair : ModuleMap)
	//{
	//	// 각 모듈을 다시 { } 로 감싸줌
	//	content += BeginBlock(tab + 2);
	//	content += AddEntry(tab + 3, "Name", pair.first); // 모듈 클래스 이름
	//	// 여기에 모듈의 상세 데이터 저장 로직 추가 가능
	//	content += EndBlock(tab + 2);
	//}
	//content += EndBlock(tab + 1); // Modules = } 닫기
	//
	//content += EndBlock(tab); // 개별 오브젝트 } 닫기
	//return content;
	return "";
}
#endif // PIXEL_EDITOR




