#pragma once
#include "PtrControlBlock.h"
#include <cstddef>
template <typename T>
class PPointer
{
public:
    // 1. 기본 생성자 및 nullptr 생성자
    PPointer(std::nullptr_t = nullptr) : cBlock(nullptr) {}

    // 2. 원본 포인터 생성자 (클래스 내부 정의 시 template <T> 중복 금지!)
    PPointer(T* ptr)
    {
        if (ptr == nullptr)
        {
            cBlock = nullptr;
            return;
        }

        if (ptr->cBlock == nullptr)
        {
            cBlock = new PtrControlBlock();
            cBlock->target = ptr;
            ptr->cBlock = cBlock;
        }
        else
        {
            cBlock = ptr->cBlock;
        }
        cBlock->AddRef();
    }

    // 3. 복사 생성자
    PPointer(const PPointer& other)
    {
        cBlock = other.cBlock;
        if (cBlock != nullptr)
        {
            cBlock->AddRef();
        }
    }

    // 4. 이동 생성자
    PPointer(PPointer&& other) noexcept
    {
        cBlock = other.cBlock;
        other.cBlock = nullptr;
    }

    // 5. 캐스팅 생성자 (U 타입을 위해 template 필요)
    template <typename U>
    PPointer(const PPointer<U>& other)
    {
        cBlock = (PtrControlBlock*)other.cBlock; // cBlock을 직접 공유
        if (cBlock != nullptr)
        {
            cBlock->AddRef();
        }
    }

    // 6. 대입 연산자
    PPointer& operator=(const PPointer& other)
    {
        if (this != &other)
        {
            if (cBlock) { cBlock->Release(); }
            cBlock = other.cBlock;
            if (cBlock) { cBlock->AddRef(); }
        }
        return *this;
    }

    // 7. 소멸자
    ~PPointer()
    {
        if (cBlock != nullptr)
        {
            cBlock->Release();
        }
    }

    // --- 유틸리티 및 연산자 ---

    T* operator -> () const
    {
        if (cBlock && cBlock->target)
            return static_cast<T*>(cBlock->target);
        return nullptr;
    }

    T* GetPtr() const
    {
        if (cBlock && cBlock->target)
            return static_cast<T*>(cBlock->target);
        return nullptr;
    }

    bool operator == (const PPointer& other) const
    {
        // 둘 다 비어있을 때 처리
        if (!cBlock && !other.cBlock) return true;
        if (!cBlock || !other.cBlock) return false;
        return cBlock->target == other.cBlock->target;
    }

    bool operator == (std::nullptr_t) const
    {
        return (cBlock == nullptr || cBlock->target == nullptr);
    }

    bool operator != (const PPointer& other) const
    {
        return !(*this == other);
    }

    PtrControlBlock* cBlock = nullptr;
};


template <typename T>
PPointer<T> MakePixel()
{
    return PPointer<T>(new T());
}

template <typename T>
PPointer<T> MakePixel(T* target)
{
    return PPointer<T>(target);
}