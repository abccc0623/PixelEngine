#pragma once
class PixelObject;
class PtrControlBlock
{
public:
	PtrControlBlock();
	~PtrControlBlock();

	void AddRefWeak();
	void AddRefShared();
	void ReleaseShared();
	void ReleaseWeak();

	int strongCount = 0;        
	int weakCount = 0;

	PixelObject* target = nullptr;
};

