#pragma once
#include "PixelEngineDLL.h"
class PtrControlBlock;
class PixelObject
{
public:
	PixelObject();
	virtual ~PixelObject();
	PtrControlBlock* cBlock = nullptr;
private:
};

