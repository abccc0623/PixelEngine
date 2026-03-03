#pragma once
#include <string>
#include <typeinfo>

using HashID = uint32_t;
constexpr HashID CalculateHash(const char* str)
{
    HashID hash = 2166136261u; // Offset Basis
    while (*str) {
        hash ^= static_cast<HashID>(*str++); // XOR
        hash *= 16777619u; // FNV Prime
    }
    return hash;
}
constexpr HashID operator "" _h(const char* str, size_t)
{
    return CalculateHash(str);
}

std::string GetTypeName(std::string name)
{
    const std::string classPrefix = "class ";
    size_t pos = name.find(classPrefix);
    if (pos != std::string::npos)
    {
        name.erase(pos, classPrefix.length());
    }
    return name;
}