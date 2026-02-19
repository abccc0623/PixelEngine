#pragma once
#include <iostream>
template <typename T>
class PPointer;
class PixelObject
{
public:
	PixelObject();
	virtual ~PixelObject();
protected:
	mutable int mRefCount = 0;
private:
	void AddRef();
	void Release();
		
	template <typename T>
	friend class PPointer;
};

template <typename T>
class PPointer
{
public:
	PPointer(std::nullptr_t = nullptr) : mPtr(nullptr) {}

	// 원본 포인터 생성자
	PPointer(T* ptr) : mPtr(ptr)
	{
		if (mPtr) mPtr->AddRef();
	}

	// 복사 생성자 (PixelObject = Object_Idle.front() 시 호출됨)
	PPointer(const PPointer& other) : mPtr(other.mPtr)
	{
		if (mPtr) mPtr->AddRef();
	}

	// 대입 연산자 (안전한 교체 로직)
	PPointer& operator=(const PPointer& other)
	{
		if (this->mPtr != other.mPtr) { // 주소가 다를 때만 처리
			if (mPtr) mPtr->Release();  // 기존 것 해제
			mPtr = other.mPtr;          // 새 포인터 복사
			if (mPtr) mPtr->AddRef();   // 새 것 카운트 증가
		}
		return *this;
	}

	PPointer(PPointer&& other) noexcept : mPtr(other.mPtr)
	{
		other.mPtr = nullptr;
	}
	
	~PPointer()
	{
		if (mPtr != nullptr) mPtr->Release();
	}

	T* operator -> ()
	{
		if (mPtr != nullptr)
		{
			return mPtr;
		}
		else 
		{
			std::cout << "nullptr" << std::endl;
			return nullptr;
		}
	}
	bool operator==(const PPointer& other) const
	{
		return mPtr == other.mPtr;
	}

	bool operator==(std::nullptr_t) const
	{
		return mPtr == nullptr;
	}

	bool operator!=(const PPointer& other) const
	{
		return mPtr != other.mPtr;
	}

	void AddRef()
	{
		if (mPtr) mPtr->AddRef();
	}

	T* GetPtr(){return mPtr;}

	template <typename U, typename... Args>
	friend PPointer<U> MakePixel(Args&&... args);
private:
	T* mPtr = nullptr;
};

template <typename T, typename... Args>
PPointer<T> MakePixel(Args&&... args)
{
	T* obj = new T(std::forward<Args>(args)...);
	return PPointer<T>(obj);
}