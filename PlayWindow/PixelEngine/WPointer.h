#pragma once
#include "PtrControlBlock.h"
#include <cstddef>
template <typename T>
class WPointer 
{
public:
    WPointer(std::nullptr_t = nullptr)
    {
        cBlock = nullptr;
    }
    WPointer(const PPointer<T>& other)
    {
        cBlock = other.cBlock;
        if (cBlock)
        {
            cBlock->AddRefWeak();
        }
    }
    WPointer(const WPointer& other)
    {
        cBlock = other.cBlock;
        if (cBlock)
        {
            cBlock->AddRefWeak();
        }
    }

    WPointer(PPointer<T>&& other)
    {
        cBlock = other.cBlock;
        if (cBlock)
        {
            cBlock->AddRefWeak();
        }
    }
    WPointer& operator=(const WPointer<T>& other)
    {
        if (this != &other)
        {
            if (cBlock) { cBlock->ReleaseWeak(); }
            cBlock = other.cBlock;
            if (cBlock) { cBlock->AddRefWeak(); }
        }
        return *this;
    }
    bool operator==(const WPointer<T>& other) const
    {
        if (cBlock == nullptr && other.cBlock == nullptr) return true;
        if (cBlock == nullptr || other.cBlock == nullptr) return false;
        if (cBlock == nullptr) return false;
        if (cBlock->target != other.cBlock->target) return false;

        return true;

    }
    bool IsValid() const
    {
        if (cBlock == nullptr) return false;
        if (cBlock->strongCount <= 0) return false;
        return true;
    }
    PPointer<T> Lock() const
    {
        return MakePixel<T>(static_cast<T*>(cBlock->target));
    }

    ~WPointer()
    {
        if (cBlock != nullptr)
        {
            cBlock->ReleaseWeak();
        }
    }
    PtrControlBlock* cBlock = nullptr;
};