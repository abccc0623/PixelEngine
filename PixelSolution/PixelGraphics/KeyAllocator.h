#pragma once
#include <cstdint>
using Handle32 = unsigned int;
using Handle64 = unsigned long long;
class KeyAllocator
{
public:
    KeyAllocator();
    ~KeyAllocator();
    std::uint16_t GetKey16();
    Handle32 Sub16(std::uint16_t num1, std::uint16_t num2);
    Handle64 Sub16(std::uint16_t num1, std::uint16_t num2, std::uint16_t num3, std::uint16_t num4);
private:
    std::uint16_t nowKey;
};