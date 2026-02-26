
#include "GameObject.h"
#include "Module.h"
#include "PixelEngine.h"
#include "BindManager.h"
#include "FunctionManager.h"
#include "ObjectManager.h"
#include <functional>
#include <iostream>
#include "Transform.h"
#include "SerializeHelper.h"
extern PixelEngine* Engine;
BindManager* GameObject::bindManager = nullptr;
FunctionManager* GameObject::functionManager = nullptr;
ObjectManager* GameObject::objectManager = nullptr;

GameObject::GameObject()
{
	ModuleMap = std::unordered_map<std::string, SPointer<Module>>();
	hashCode = std::hash<GameObject*>{}(this);

	if (bindManager == nullptr)
	{
		bindManager = Engine->GetFactory<BindManager>();
	}
	if (functionManager == nullptr)
	{
		functionManager = Engine->GetFactory<FunctionManager>();
	}
	if (objectManager == nullptr)
	{
		objectManager = Engine->GetFactory<ObjectManager>();
	}
}
GameObject::~GameObject()
{
	ModuleMap.clear();
}

size_t GameObject::GetHashCode()
{
	return hashCode;
}

std::vector<SPointer<Module>> GameObject::GetModules()
{
	std::vector<SPointer<Module>> list = std::vector<SPointer<Module>>();
	for (auto K : ModuleMap)
	{
		list.push_back(K.second);
	}
	return list;
}

void GameObject::Destroy()
{
	
}

void GameObject::OnCollision2D(WPointer<GameObject> target)
{
	for (auto& K : ModuleMap)
	{
		if (K.second->isCollisionModule == true)continue;
		K.second->OnCollision2D(target);
	}
}

std::string GameObject::Save(int tab)
{
	std::string content = BeginBlock(tab); // 개별 오브젝트 { 시작
	content += AddEntry(tab + 1, "Name", name);
	content += AddEntry(tab + 1, "HashCode", (float)hashCode);

	content += BeginBlock(tab + 1, "Modules"); // Modules = { 시작
	for (auto& pair : ModuleMap)
	{
		// 각 모듈을 다시 { } 로 감싸줌
		content += BeginBlock(tab + 2);
		content += AddEntry(tab + 3, "Name", pair.first); // 모듈 클래스 이름
		// 여기에 모듈의 상세 데이터 저장 로직 추가 가능
		content += EndBlock(tab + 2);
	}
	content += EndBlock(tab + 1); // Modules = } 닫기

	content += EndBlock(tab); // 개별 오브젝트 } 닫기
	return content;
}

