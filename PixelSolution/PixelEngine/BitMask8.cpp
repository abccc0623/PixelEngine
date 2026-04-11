#include "pch.h"
#include "BitMask8.h"

BitMask8::BitMask8() : bits(0) {}

BitMask8::BitMask8(uint8_t initialValue) : bits(initialValue) {}

// 비트 켜기 (예: set(MOVE | START))
void BitMask8::set(uint8_t mask) {
    bits |= mask;
}

// 비트 끄기 (예: reset(START))
void BitMask8::reset(uint8_t mask) {
    bits &= ~mask;
}

// 비트 반전
void BitMask8::flip(uint8_t mask) {
    bits ^= mask;
}

// 전달받은 비트 중 하나라도 켜져 있는지 확인
bool BitMask8::hasAny(uint8_t mask) const {
    return (bits & mask) != 0;
}

// 전달받은 비트가 모두 켜져 있는지 확인
bool BitMask8::hasAll(uint8_t mask) const {
    return (bits & mask) == mask;
}

void BitMask8::clear() {
    bits = 0;
}

uint8_t BitMask8::getValue() const {
    return bits;
}

void BitMask8::setValue(uint8_t newValue) {
    bits = newValue;
}