#include "PixelObject.h"

PixelObject::PixelObject()
{

}

PixelObject::~PixelObject()
{

}

void PixelObject::AddRef()
{
	mRefCount++;
}

void PixelObject::Release()
{
	mRefCount--;
	if (mRefCount <= 0)
	{
		delete this;
	}
}
