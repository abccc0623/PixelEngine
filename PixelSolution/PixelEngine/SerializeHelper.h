#pragma once
#include <string>

std::string GetTabs(int tab);
// 2. 문자열 Value용 (std::quoted 사용)
std::string AddEntry(int tab, const std::string& key, const std::string& value);
// 3. 숫자(float/int) Value용
std::string AddEntry(int tab, const std::string& key, float value);
// 4. 불리언 Value용
std::string AddEntry(int tab, const std::string& key, bool value);

std::string BeginBlock(int tab, const std::string& key = "");

std::string EndBlock(int tab, bool comma = true);
