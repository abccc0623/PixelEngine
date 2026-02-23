#include "PtrControlBlock.h"
#include "PixelObject.h"
#include <iostream>
PtrControlBlock::PtrControlBlock()
{

}
PtrControlBlock::~PtrControlBlock()
{
	
}
void PtrControlBlock::AddRefWeak()
{
	weakCount++;
}
void PtrControlBlock::AddRefShared()
{
	strongCount++;
}

void PtrControlBlock::ReleaseShared()
{
	strongCount--;
	if (strongCount <= 0 && target != nullptr)
	{
		if (target != nullptr)
		{
			printf("Release: S:%d, W:%d\n", strongCount, weakCount);
			delete target;
			target = nullptr;
		}
	}

	if (strongCount <= 0 && weakCount <= 0)
	{
		delete this;
	}
}

void PtrControlBlock::ReleaseWeak()
{
	weakCount--;
	if (strongCount == 0 && weakCount == 0)
	{
		printf("Release: S:%d, W:%d\n", strongCount, weakCount);
		delete this;
	}
}
