#pragma once
#include <cstdint>

class BitMask8 {
private:
	uint8_t bits; // 내부 저장 데이터

public:
	BitMask8();
	BitMask8(uint8_t initialValue);

	// 비트 제어 (인자가 1, 2, 4, 8... 같은 비트값임)
	void set(uint8_t mask);      // 비트 켜기
	void reset(uint8_t mask);    // 비트 끄기
	void flip(uint8_t mask);     // 비트 반전

	// 비트 확인
	bool hasAny(uint8_t mask) const; // 하나라도 포함되었는가? (mask & bits) != 0
	bool hasAll(uint8_t mask) const; // 모두 포함되었는가?   (mask & bits) == mask

	void clear();

	// Getter / Setter
	uint8_t getValue() const;
	void setValue(uint8_t newValue);
};