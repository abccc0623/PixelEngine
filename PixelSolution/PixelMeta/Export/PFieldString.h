#pragma once
#include "PixelMetaAPI.h"


template<typename ClassType>
class PFieldString : public PFieldTemplate <std::string, ClassType>
{
public:
	PFieldString(const std::string& fieldName, std::string ClassType::* memberPtr)
		: PFieldTemplate<std::string, ClassType>(fieldName, memberPtr) {}

	virtual ~PFieldString() {};

    void SetValue(void* instance, void* valuePtr) override
    {
        if (!instance || !valuePtr) return;

        // 1. 대상 객체의 string 주소를 찾습니다.
        std::string* target = reinterpret_cast<std::string*>(reinterpret_cast<char*>(instance) + this->offset);

        // 2. 넘겨받은 값도 string 포인터여야 합니다.
        std::string* source = reinterpret_cast<std::string*>(valuePtr);

        // 3. [중요] 단순 메모리 복사가 아니라, string의 대입 연산자를 호출합니다.
        // 이렇게 해야 내부의 동적 할당 메모리가 안전하게 복사됩니다.
        *target = *source;
    }
};

