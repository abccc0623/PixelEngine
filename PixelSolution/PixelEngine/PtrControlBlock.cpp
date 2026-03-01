#include "pch.h"
#include "PtrControlBlock.h"
#include "Core/PixelObject.h"
#include "Log.h"
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
			Log::Info("Release:" + std::to_string(strongCount) + "," + std::to_string(weakCount));
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
		Log::Info("Release:" + std::to_string(strongCount) + "," + std::to_string(weakCount));
		delete this;
	}
}
