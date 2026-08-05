#include "pch.h"
#include "Transform.h"
#include "PixelEngineAPI.h"
#include "Registry.h"
#include "EntityObject.h"
TransformData* Transform_Add(unsigned int id)
{
	auto registry = GetRegistry();
	registry->Add<TransformData>(id);
	registry->Add<WorldData>(id);

	auto data = registry->Get<TransformData>(id);
	data->thisID = id;
	return data;
}

TransformData* Transform_Get(unsigned int id)
{
	auto registry = GetRegistry();
	TransformData* data = registry->Get<TransformData>(id);
	if (data == nullptr)
	{
		PixelLog::Error("[Transform][GetComponent] Not Find Component");
	}
	return data;
}

bool Transform_Has(unsigned int id)
{
	auto registry = GetRegistry();
	TransformData* data = registry->Get<TransformData>(id);
	if (data == nullptr)
	{
		return false;
	}
	else
	{
		return true;
	}
}

void Transform_SetParent(unsigned int id, unsigned int ParentID)
{
	auto thisEntity = FindEntity(id);
	if (thisEntity == nullptr)
	{
		PixelLog::Error("[Transform][SetParent] Not Find This EntityID :" + id);
		return;
	}
	thisEntity->SetParent(ParentID);
}

void Transform_SetChild(unsigned int id, unsigned int ChildID)
{
	auto thisEntity = FindEntity(id);
	if (thisEntity == nullptr)
	{
		PixelLog::Error("[Transform][SetParent] Not Find This EntityID :" + id);
		return;
	}
	thisEntity->SetChild(ChildID);

}

void Transform_SetLocalPosition(unsigned int id, PVector3 position)
{


}

void Transform_SetLocalRotation(unsigned int id, PVector3 rotation)
{

}

void Transform_SetLocalScale(unsigned int id, PVector3 scale)
{

}
