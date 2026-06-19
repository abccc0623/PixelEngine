#pragma once


namespace ECS::Pool
{
	unsigned int CreateEntity(const char* poolName, const char* scriptName);
	void Push(const char* poolName, unsigned int id);
	void Pop(const char* poolName);
}