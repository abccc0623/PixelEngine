#include "pch.h"
#include "Memory.h"
#include <cstdlib>

void* operator new(size_t size, const char* file, int line)
{
    void* ptr = std::malloc(size);
    if (ptr) {
        MemoryTracker::Get().Add(ptr, size, file, line);
    }
    return ptr;
}

void operator delete(void* ptr) noexcept
{
    MemoryTracker::Get().Remove(ptr);
    std::free(ptr);
}

void operator delete(void* ptr, const char* file, int line) noexcept 
{
    MemoryTracker::Get().Remove(ptr);
    std::free(ptr);
}