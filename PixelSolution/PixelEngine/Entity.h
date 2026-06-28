#pragma once

#include <sol/forward.hpp>
namespace ECS::Entity
{
	//생성,삭제
	unsigned int Create(const char* scriptName);
	void Destroy(unsigned int id);
	void SetActive(unsigned int id, bool active);
	bool GetActive(unsigned int id);



	sol::object GetValue(unsigned int id, const char* memberName);
	void SetValue(unsigned int id, const char* memberName, sol::object);
}