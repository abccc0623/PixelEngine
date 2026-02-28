#pragma once
#include "PtrControlBlock.h"
#include <cstddef>
#include <utility>
template <typename T>
class SPointer
{
public:
    // 1. 기본 생성자 및 nullptr 생성자
    SPointer(std::nullptr_t = nullptr) : cBlock(nullptr) {}

    // 2. 원본 포인터 생성자 (클래스 내부 정의 시 template <T> 중복 금지!)
    SPointer(T* ptr)
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
        cBlock->AddRefShared();
    }

    // 3. 복사 생성자
    SPointer(const SPointer& other)
    {
        cBlock = other.cBlock;
        if (cBlock != nullptr)
        {
            cBlock->AddRefShared();
        }
    }

    // 4. 이동 생성자
    SPointer(SPointer&& other) noexcept
    {
        cBlock = other.cBlock;
        other.cBlock = nullptr;
    }

    // 5. 캐스팅 생성자 (U 타입을 위해 template 필요)
    template <typename U>
    SPointer(const SPointer<U>& other)
    {
        cBlock = (PtrControlBlock*)other.cBlock; // cBlock을 직접 공유
        if (cBlock != nullptr)
        {
            cBlock->AddRefShared();
        }
    }

    // 6. 대입 연산자
    SPointer& operator=(const SPointer& other)
    {
        if (this != &other)
        {
            if (cBlock) { cBlock->ReleaseShared(); }
            cBlock = other.cBlock;
            if (cBlock) { cBlock->AddRefShared(); }
        }
        return *this;
    }

    // 7. 소멸자
    ~SPointer()
    {
        if (cBlock != nullptr)
        {
            cBlock->ReleaseShared();
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

    bool operator == (const SPointer& other) const
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

    bool operator != (const SPointer& other) const
    {
        return !(*this == other);
    }

    template <typename... Args>
    static SPointer<T> Make_SPointer(Args&&... args)
    {
        return SPointer<T>(new T());
    }

    PtrControlBlock* cBlock = nullptr;
};


//template <typename T>
//SPointer<T> MakePixel()
//{
//    return SPointer<T>(new T());
//}
//
