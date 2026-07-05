#pragma once
#include <string>
#include <vector>
#include "PixelMetaAPI.h"
struct PixelParameterMeta
{
	std::string type;
	std::string name;
	std::string tooltip;
};

struct PixelMemberMeta
{
	std::string name;
	std::string type;
	bool luaBind = false;
};

struct PixelMethodMeta
{
	std::string name;
	std::string returnType;
	std::string tooltip;
	std::vector<PixelParameterMeta> propertys;
};

struct PixelEnumMeta
{
	std::string name;
	std::vector<std::string> value;
};

struct PixelClassMeta
{
	long flag;
	std::string name;
	std::string type;
	std::vector<PixelMemberMeta> members;
	std::vector<PixelMethodMeta> methods;
};
struct PixelClassDataMeta
{
	std::string name;
	std::string type;
	std::vector<PixelMemberMeta> members;
	std::vector<PixelMethodMeta> methods;
};





struct PixelStaticMeta
{
	std::string name;
	std::string type;
	std::vector<PixelMethodMeta> methods;
};

struct PixelTypes
{
	std::vector<PixelStaticMeta> staticList;
	std::vector<PixelClassMeta> classList;
	std::vector<PixelEnumMeta>	enumList;
};

struct PixelComponent
{
	PixelClassMeta classComponent;
	PixelStaticMeta staticComponent;
};
