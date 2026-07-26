#include "pch.h"
#include "KeyAllocator.h"

KeyAllocator::KeyAllocator() :
    nowKey(0)
{

}
KeyAllocator::~KeyAllocator()
{

}

std::uint16_t KeyAllocator::GetKey16()
{
    nowKey++;
    std::bitset<16> b(nowKey);
    std::cout << "RenderKey (Binary): " << b << std::endl;
    return nowKey;
}
Handle32 KeyAllocator::Sub16(std::uint16_t num1, std::uint16_t num2)
{
    Handle32 tabel;
    tabel = num1;
    tabel <<= 8;
    tabel |= (num2 & 0xFF);
    std::bitset<32> b(tabel);
    std::cout << "RenderKey (Binary): " << b << std::endl;
    return tabel;
}

Handle64 KeyAllocator::Sub16(std::uint16_t num1, std::uint16_t num2, std::uint16_t num3, std::uint16_t num4)
{
    return Handle64();
}
