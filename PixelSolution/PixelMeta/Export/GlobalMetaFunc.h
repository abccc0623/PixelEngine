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


namespace HashUtil
{
    constexpr uint64_t FNV_OFFSET_BASIS = 14695981039346656037ULL;
    constexpr uint64_t FNV_PRIME = 1099511628211ULL;

    constexpr uint64_t ConstexprHash(const char* str, uint64_t hash = FNV_OFFSET_BASIS) {
        return (*str == '\0') ? hash : ConstexprHash(str + 1, (hash ^ static_cast<uint64_t>(*str)) * FNV_PRIME);
    }
}


template <typename T>
struct TypeHash {
    static constexpr uint64_t Value() {
        // 컴파일러별 고유 함수 시그니처 매크로 사용
#if defined(_MSC_VER)
        return HashUtil::ConstexprHash(__FUNCSIG__);
#elif defined(__clang__) || defined(__GNUC__)
        return HashUtil::ConstexprHash(__PRETTY_FUNCTION__);
#else
#error "지원하지 않는 컴파일러입니다."
#endif
    }
};