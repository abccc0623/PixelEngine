#include "SerializeHelper.h"
#include <iomanip>
#include <sstream>
// 탭 생성
std::string GetTabs(int tab) {
    return std::string(tab, '\t');
}

// 일반 값 추가 (문자열)
std::string AddEntry(int tab, const std::string& key, const std::string& value) 
{
    std::stringstream ss;
    ss << GetTabs(tab) << key << " = " << std::quoted(value) << ",\n";
    return ss.str();
}

// 일반 값 추가 (숫자)
std::string AddEntry(int tab, const std::string& key, float value)
{
    std::stringstream ss;
    // 숫자가 너무 크게 지수로 나오지 않게 고정 (8.70145e+18 방지)
    ss << std::fixed << std::setprecision(0);
    ss << GetTabs(tab) << key << " = " << value << ",\n";
    return ss.str();
}

// 테이블 시작 함수
std::string BeginBlock(int tab, const std::string& key)
{
    if (key.empty()) return GetTabs(tab) + "{\n";
    return GetTabs(tab) + key + " = {\n";
}

// 테이블 종료 함수
std::string EndBlock(int tab, bool comma) {
    return GetTabs(tab) + (comma ? "},\n" : "}\n");
}