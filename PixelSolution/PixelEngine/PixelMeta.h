#pragma once
#include <string>
#include <vector>
#include "PixelMetaAPI.h"

struct PixelMemberMeta
{
	std::string name;
	std::string type;
};

struct PixelMethodMeta
{
	std::string name;
	std::string returnType;
	std::vector<std::string> propertys;
};

struct PixelClassMeta
{
	META_TYPE metaType;
	uint64_t parentHash;
	std::string thisName;
	std::vector<PixelMemberMeta> members;
	std::vector<PixelMethodMeta> methods;
};