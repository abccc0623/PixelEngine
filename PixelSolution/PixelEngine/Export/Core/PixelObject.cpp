#include "pch.h"
#include "PixelObject.h"
#include "PtrControlBlock.h"

PixelObject::PixelObject()
{

}

PixelObject::~PixelObject()
{
	if (cBlock != nullptr)
	{
		cBlock->target = nullptr;
	}
}


