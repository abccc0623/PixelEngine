#include "PixelObject.h"

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


