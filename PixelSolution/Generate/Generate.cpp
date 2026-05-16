// Generate.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//
#include "PixelEngineAPI.h"
#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;
std::string GetExePath() {
	char buffer[MAX_PATH];
	GetModuleFileNameA(NULL, buffer, MAX_PATH);
	return fs::path(buffer).parent_path().string();
}

int main()
{
	EngineInitialize(nullptr, 100, 100);

	while (true)
	{
		std::string inputStr;

		// 1. 안내 문구 출력 (이때 std::cout 사용)
		std::cout << "생성할 번호를 입력" << std::endl;
		std::cout << "(1)LuaGenerate" << std::endl;
		std::cout << "(2)LSPGenerate" << std::endl;
		std::cout << "(3)LSPGenerate" << std::endl;
		std::cout << "(4)종료" << std::endl;

		// 2. 공백 포함 한 줄 전체를 입력받음 (std::getline 사용)
		std::getline(std::cin, inputStr);
		int number = 0;
		try
		{
			number = std::stoi(inputStr);
			std::string outputPath;
			switch (number)
			{
			case 1:
				outputPath = "../PixelEngine/GenerateLuaBind.h";
				LuaGenerate(outputPath.c_str());
				break;
			case 2:
				outputPath += "../PixelTool/LSP/bin/GenerateLuaAPI.lua";
				LSPGenerate(outputPath.c_str());
				break;
			case 3:
				JsonGenerate(outputPath.c_str());
				//outputPath = "../PixelEngine/GenerateLuaBind.h";
				//GenerateLuaBindCode(outputPath.c_str());
				//outputPath = GetExePath();
				//outputPath += "\\LSP\\bin\\GenerateLuaAPI.lua";
				//GenerateLuaAPICodeJson(outputPath.c_str());
				return 0;
			}
		}
		catch (const std::invalid_argument& e)
		{
			// 사용자가 "abc" 같은 걸 쳤을 때 방어
			std::cout << "[에러] 문자가 아닌 숫자를 입력해야 합니다!\n";
			return -1; // 에러 코드 반환
		}
		catch (const std::out_of_range& e)
		{
			// 너무 큰 숫자를 쳤을 때 방어
			std::cout << "[에러] int 범위를 벗어난 너무 큰 숫자입니다!\n";
			return -1;
		}
	}
}
