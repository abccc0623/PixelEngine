#pragma once
using Handle16 = unsigned short;
using Handle32 = unsigned int;
using Handle64 = unsigned long long;
class KeyAllocator
{
public:
    KeyAllocator();
    ~KeyAllocator();
    Handle16 GetKey16();
    Handle32 Sub16(Handle16 num1, Handle16 num2);
    Handle64 Sub16(Handle16 num1, Handle16 num2, Handle16 num3, Handle16 num4);
private:
    Handle16 nowKey;
};