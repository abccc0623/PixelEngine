#include "pch.h"
#include "Transform.h"
#include "PixelEngineAPI.h"
#include "Registry.h"
TransformData* Transform_Add(unsigned int id)
{
	auto registry = GetRegistry();
	registry->Add<TransformData>(id);
	registry->Add<WorldData>(id);
	return registry->Get<TransformData>(id);
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
