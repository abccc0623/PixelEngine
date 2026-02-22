#pragma once
class PixelObject;
class PtrControlBlock
{
public:
	PtrControlBlock();
	~PtrControlBlock();

	void AddRefWeak();
	void AddRef();
	void Release();
	void ReleaseWeak();


	PixelObject* target = nullptr;
	int strongCount = 0;        
	int weakCount = 0;
};

