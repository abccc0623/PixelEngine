#include "KeyAllocator.h"
#include <iostream>
#include <iomanip>
#include <bitset>

KeyAllocator::KeyAllocator() :
    nowKey(0)
{

}
KeyAllocator::~KeyAllocator()
{

}

Handle16 KeyAllocator::GetKey16()
{
    nowKey++;
    std::bitset<16> b(nowKey);
    std::cout << "RenderKey (Binary): " << b << std::endl;
    return nowKey;
}
Handle32 KeyAllocator::Sub16(Handle16 num1, Handle16 num2)
{
    Handle32 tabel;
    tabel = num1;
    tabel <<= 8;
    tabel |= (num2 & 0xFF);
    std::bitset<32> b(tabel);
    std::cout << "RenderKey (Binary): " << b << std::endl;
    return tabel;
}

Handle64 KeyAllocator::Sub16(Handle16 num1, Handle16 num2, Handle16 num3, Handle16 num4)
{
    return Handle64();
}
