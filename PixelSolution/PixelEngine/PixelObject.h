#pragma once
#include"PtrControlBlock.h"
#include "SPointer.h"
#include "WPointer.h"
#include "UPointer.h"
#include <iostream>
#include <type_traits>
template <typename T> class SPointer;
class PixelObject
{
public:
	PixelObject();
	virtual ~PixelObject();
private:
	PtrControlBlock* cBlock = nullptr;
		
	template <typename T>
	friend class SPointer;
	template <typename U>
	friend SPointer<U> MakePixel();
	template <typename U>
	friend SPointer<U> MakePixel(U* target);
};

