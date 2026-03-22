#pragma once
#include <string>
#include <typeinfo>

enum MetaFlag : long
{
    None    = 0,
    SAVE    = 1 << 0,  // 1
    LUABIND = 1 << 1,  // 2
    Flag3   = 1 << 2,  // 4
    Flag4   = 1 << 3,  // 8
    Flag5   = 1 << 4,
    Flag6   = 1 << 5,
    Flag7   = 1 << 6,
    Flag8   = 1 << 7   // 128
};

enum class META_TYPE
{
    PRIMITIVE,
    NAMESPACE,
    CLASS,
    ENUM,
    ARRAY,
};

namespace HashUtil
{
    constexpr uint64_t FNV_OFFSET_BASIS = 14695981039346656037ULL;
    constexpr uint64_t FNV_PRIME = 1099511628211ULL;

    // 런타임/컴파일 타임 공용 64비트 해시 함수
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

template<typename T>
std::string ExtractTypeName() {
#if defined(__clang__) || defined(__GNUC__)
    std::string sig = __PRETTY_FUNCTION__;
    size_t start = sig.find("T = ") + 4;
    size_t end = sig.find_last_of(']');
    return sig.substr(start, end - start);

#elif defined(_MSC_VER)
    std::string sig = __FUNCSIG__;
    size_t start = sig.find("ExtractTypeName<") + 16;
    size_t end = sig.find_last_of('>');
    std::string name = sig.substr(start, end - start);

    // MSVC 특유의 찌꺼기 제거
    size_t pos = name.find("class ");
    if (pos != std::string::npos) name.erase(pos, 6);
    pos = name.find("struct ");
    if (pos != std::string::npos) name.erase(pos, 7);
    return name;
#else
    return "UnknownType";
#endif
}
template<>
inline std::string ExtractTypeName<std::string>() {
    return "std::string";
}

