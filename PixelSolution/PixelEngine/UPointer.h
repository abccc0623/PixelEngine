#pragma once
#include <cstddef>
#include <utility>
template <typename T>
class UPointer
{
public:
	UPointer(std::nullptr_t = nullptr) : p(nullptr) {}
	explicit UPointer(T* ptr)
	{
		p = ptr;
	}
	UPointer(const UPointer&) = delete;

	UPointer(UPointer&& other) noexcept : p(other.p)
	{
		other.p = nullptr;
	}

	~UPointer()
	{
		if(p){delete p;}
	}
	
	T* operator->() const { return p; }
	T& operator*() const { return *p; }

	UPointer& operator=(const UPointer&) = delete;

	UPointer& operator=(UPointer&& other) noexcept
	{
		if (this != &other) 
		{
			if (p != nullptr) delete p;
			p = other.p;
			other.p = nullptr;
		}
		return *this;
	}

	template <typename... Args>
	static UPointer<T> Make_UPointer(Args&&... args)
	{
		return UPointer<T>(new T(std::forward<Args>(args)...));
	}
	T* p;
};