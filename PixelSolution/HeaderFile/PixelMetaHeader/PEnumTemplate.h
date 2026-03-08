#pragma once
#include "PixelMetaDLL.h"
#include "GlobalHashCode.h"
#include "PEnum.h"
template<typename ThisType>
class PEnumTemplate : public PEnum 
{
public:
	PEnumTemplate()
	{
		typeName = ExtractTypeName<ThisType>();
		typeHash = HashUtil::ConstexprHash(typeName.c_str());
		typeSize = 0;
		metaType = META_TYPE::ENUM;
	}
	virtual ~PEnumTemplate() {};


};

