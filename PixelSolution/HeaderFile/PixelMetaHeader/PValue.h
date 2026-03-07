#pragma once
#include <string>
#include <unordered_map>
#include "PixelMetaDLL.h"
class PType;
class PValue
{
public:
    PIXEL_META_DLL PValue();
    PIXEL_META_DLL PValue(int32_t val);
    PIXEL_META_DLL PValue(float val);
    PIXEL_META_DLL PValue(bool val);
    PIXEL_META_DLL PValue(char val);
    PIXEL_META_DLL PValue(double val);
    PIXEL_META_DLL PValue(void* ptr);

    template<typename T>
    PValue(const T& val) {
        // 여기서 엔진의 타입 시스템과 매칭하는 로직을 넣습니다.
        this->Type = GetTypeByKeyword<T>();
        // 데이터를 보관하는 로직 (예: void*에 할당하거나 내부 버퍼에 복사)
        //this->Data = new T(val);
    }

	PIXEL_META_DLL ~PValue();

    PIXEL_META_DLL int32_t          AsInt();
    PIXEL_META_DLL float            AsFloat();
    PIXEL_META_DLL bool             AsBool();
    PIXEL_META_DLL char             AsChar();
    PIXEL_META_DLL double           Asdouble();
    PIXEL_META_DLL std::string      AsString();
    PIXEL_META_DLL void*            AsPointer();

    union 
    {
        int32_t  v_int;
        float    v_float;
        bool     v_bool;
        char     v_char;
        double   v_double;
        void*    v_ptr;
        uint64_t v_raw; // 초기화용
    } Data;
private:
    PType* Type;
    std::string value;
};

