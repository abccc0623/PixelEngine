#include "pch.h"
#include "LuaManager.h"
#include "PixelEngine.h"
#include "PixelEngineAPI.h"
#include "Core/GameObject.h"
#include "Core/Module.h"
#include "resource.h"
#include "ResourceManager.h"
#include "Module/Transform.h"
#include "BindManager.h"
#include "KeyInputManager.h"
#include "SceneManager.h"
#include "SPointer.h"

#include "LuaModuleInfo.h"
#include "LuaSceneInfo.h"
#include "PixelMetaAPI.h"
#include "CoroutineManager.h"

#include "Module/Renderer2D.h"
#include "Module/LuaScript.h"
#include "GenerateLuaBind.h"



#define SOL_ALL_SAFETIES_ON 1 // 안전장치 활성화 (권장)


extern PixelEngine* Engine;
LuaManager::LuaManager()
{
	luaModuleTableMap = std::unordered_map<std::string, LuaModuleInfo*>();

#ifdef LUAJIT_VERSION
	std::cout << "컴파일 시점 확인: LuaJIT용으로 빌드됨!" << std::endl;
#else
	std::cout << "컴파일 시점 확인: 일반 Lua용으로 빌드됨!" << std::endl;
#endif
}

LuaManager::~LuaManager()
{

}
void LuaManager::Initialize()
{
	lua = sol::state();
	lua.open_libraries(
		sol::lib::base,    // print, assert, type 등 기본 함수
		sol::lib::package, // require, package.path 등 (첫 번째 에러 해결)
		sol::lib::table,   // table.insert, table.remove 등 (두 번째 에러 해결)
		sol::lib::string,  // 문자열 처리
		sol::lib::math,    // 수학 연산
		sol::lib::os,      // 시간(os.time) 등 시스템 함수
		sol::lib::debug,    // 디버깅 툴
		sol::lib::jit,     // [추가됨] LuaJIT 컨트롤 (jit.on, jit.off, 최적화 옵션 등)
		sol::lib::ffi
	);
	bind = Engine->GetFactory<BindManager>();
	input = Engine->GetFactory<KeyInputManager>();

	//BindLuaKey();
	//BindLuaTime();
	BindAll_GeneratedLuaModules(lua);
}

LuaModuleInfo* LuaManager::GetModuleLua(const std::string& fileName)
{
	auto k = luaModuleTableMap.find(fileName);
	if (k != luaModuleTableMap.end())
	{
		return k->second;
	}
	return nullptr;
}

LuaSceneInfo* LuaManager::GetSceneLua(const std::string& fileName)
{
	auto k = luaSceneTableMap.find(fileName);
	if (k != luaSceneTableMap.end())
	{
		return k->second;
	}
	return nullptr;
}

std::string LuaManager::ChangeLuaType(std::string type)
{
	if (type == "int" || type == "float")
	{
		return "number";
	}
	else if (type == "std::basic_string<char,std::char_traits<char>,class std::allocator<char> >")
	{
		return "string";
	}
	else
	{
		return type;
	}
}


void LuaManager::ImportLua(const std::string& filePath, const std::string filename, const std::string& ext)
{
	try
	{
		if (filename == "main")
		{
			sol::protected_function_result result = lua.script_file(filePath);
			sol::protected_function mainFunc = lua["Main"];
			if (mainFunc.valid())
			{
				auto result = mainFunc();
				if (result.valid() == false)
				{
					sol::error err = result;
					std::string what = err.what();
					PixelLog::Error("mina 스크립트 로드 실패: " + what);
				}
			}
		}
		else if (ext == ".scene")
		{
			sol::protected_function_result result = lua.script_file(filePath);
			if (result.return_count() > 0 && result[0].is<sol::table>())
			{
				if (result.valid())
				{
					sol::table Proto = result[0];
					luaSceneTableMap.insert({ filename , new LuaSceneInfo(Proto) });
					SceneManager* scene = Engine->GetFactory<SceneManager>();
					scene->CreateScene(filePath);
				}
			}
		}
		else if (ext == ".pxm")
		{
			// 환경을 먼저 만들고
			sol::environment prototypeEnv(lua, sol::create, lua.globals());

			// 해당 환경에 Wait를 직접 등록 (이래야 루아가 멈춥니다)
			prototypeEnv.set_function("WaitForSeconds", sol::yielding([this](float seconds, sol::this_state s)
				{
					CoroutineManager* cm = Engine->GetFactory<CoroutineManager>();
					if (cm)
					{
						cm->MarkAsWaiting(s, seconds);
					}
				}));

			sol::protected_function_result result = lua.script_file(filePath, prototypeEnv);
			if (result.valid() && result.get_type() == sol::type::table)
			{
				sol::table blueprint = result;
				luaModuleTableMap.insert({ filename, new LuaModuleInfo(blueprint) });
			}
			else { /* 에러 처리 */ }
		}
	}
	catch (const sol::error& e)
	{
		PixelLog::Error(e.what());
	}
}

void LuaManager::BindLuaKey()
{
	//Key관련 등록
	sol::table input = lua.create_named_table("Input");
	input["GetKey"] = [](int keyNumber)->bool { return GetKey(keyNumber); };
	input["GetKeyDown"] = [](int keyNumber)->bool {return GetKeyDown(keyNumber); };
	input["GetKeyUp"] = [](int keyNumber)->bool { return GetKeyUp(keyNumber); };
	input["GetMousePosition_X"] = GetMousePosition_X;
	input["GetMousePosition_Y"] = GetMousePosition_Y;
}
void LuaManager::BindLuaTime()
{
	sol::table time = lua.create_named_table("Time");
	time["GetDeltaTime"] = GetDeltaTime;
	time["GetTotalTime"] = GetTotalTime;
	time["GetFPS"] = GetFPS;
}



std::string LuaManager::SettingKeyEnum()
{
	std::string main = "";
	main += "---@enum KeyCode\n"; // EmmyLua 자동완성을 위한 어노테이션
	main += "KeyCode = {\n";

	// 마우스 및 특수키
	main += "    LButton = 0x01, RButton = 0x02, Cancel = 0x03, MButton = 0x04, \n";
	main += "    Backspace = 0x08, Tab = 0x09, Clear = 0x0C, Enter = 0x0D, \n";
	main += "    Shift = 0x10, Control = 0x11, Alt = 0x12, Pause = 0x13, CapsLock = 0x14, \n";
	main += "    Escape = 0x1B, Space = 0x20, PageUp = 0x21, PageDown = 0x22, End = 0x23, Home = 0x24, \n";

	// 방향키
	main += "    Left = 0x25, Up = 0x26, Right = 0x27, Down = 0x28, \n";
	main += "    Select = 0x29, Print = 0x2A, Execute = 0x2B, PrintScreen = 0x2C, Insert = 0x2D, Delete = 0x2E, \n";

	// 숫자키 (0-9)
	main += "    Alpha0 = 0x30, Alpha1 = 0x31, Alpha2 = 0x32, Alpha3 = 0x33, Alpha4 = 0x34, \n";
	main += "    Alpha5 = 0x35, Alpha6 = 0x36, Alpha7 = 0x37, Alpha8 = 0x38, Alpha9 = 0x39, \n";

	// 알파벳 (A-Z)
	main += "    A = 0x41, B = 0x42, C = 0x43, D = 0x44, E = 0x45, F = 0x46, G = 0x47, H = 0x48, \n";
	main += "    I = 0x49, J = 0x4A, K = 0x4B, L = 0x4C, M = 0x4D, N = 0x4E, O = 0x4F, P = 0x50, \n";
	main += "    Q = 0x51, R = 0x52, S = 0x53, T = 0x54, U = 0x55, V = 0x56, W = 0x57, X = 0x58, \n";
	main += "    Y = 0x59, Z = 0x5A, \n";

	// 넘패드
	main += "    Numpad0 = 0x60, Numpad1 = 0x61, Numpad2 = 0x62, Numpad3 = 0x63, Numpad4 = 0x64, \n";
	main += "    Numpad5 = 0x65, Numpad6 = 0x66, Numpad7 = 0x67, Numpad8 = 0x68, Numpad9 = 0x69, \n";
	main += "    Multiply = 0x6A, Add = 0x6B, Separator = 0x6C, Subtract = 0x6D, Decimal = 0x6E, Divide = 0x6F, \n";

	// 기능키 (F1-F12)
	main += "    F1 = 0x70, F2 = 0x71, F3 = 0x72, F4 = 0x73, F5 = 0x74, F6 = 0x75, \n";
	main += "    F7 = 0x76, F8 = 0x77, F9 = 0x78, F10 = 0x79, F11 = 0x7A, F12 = 0x7B, \n";

	main += "}\n\n";
	//lua->script(main);
	return main;
}

void LuaManager::Update()
{

}

void LuaManager::Release()
{
	for (auto& k : luaModuleTableMap)
	{
		delete k.second;
	}
	for (auto& k : luaSceneTableMap)
	{
		delete k.second;
	}
	luaModuleTableMap.clear();
	luaSceneTableMap.clear();
	lua.collect_garbage();
}

void LuaManager::Clear()
{
	Release();
}
