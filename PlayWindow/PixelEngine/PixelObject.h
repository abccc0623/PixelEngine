#pragma once
#include"PtrControlBlock.h"
#include "PPointer.h"
#include "WPointer.h"
#include <iostream>
#include <type_traits>
template <typename T> class PPointer;
class PixelObject
{
public:
	PixelObject();
	virtual ~PixelObject();
private:
	PtrControlBlock* cBlock = nullptr;
		
	template <typename T>
	friend class PPointer;
	template <typename U>
	friend PPointer<U> MakePixel();
	template <typename U>
	friend PPointer<U> MakePixel(U* target);
};

