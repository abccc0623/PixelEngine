#include "pch.h"
#include "LuaManager.h"
#include "PixelEngine.h"
#include "PixelEngineAPI.h"
#include "Core/Module.h"
#include "resource.h"
#include "ResourceManager.h"
#include "SceneManager.h"
#include "SceneObject.h"
#include "Entity.h"
#include "Pool.h"

#include "LuaModuleInfo.h"
#include "LuaSceneInfo.h"
#include "PixelMetaAPI.h"
#include "CoroutineManager.h"
#include "LuaDebugger.h"

#include <filesystem>
#include "Debug.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define GET_CURRENT_MODULE() ((HMODULE)&__ImageBase)

extern PixelEngine* Engine;

namespace
{
	std::filesystem::path EngineApiDirectory()
	{
		std::wstring executable(32768, L'\0');
		const DWORD length = GetModuleFileNameW(nullptr, executable.data(), static_cast<DWORD>(executable.size()));
		if (length == 0 || length >= executable.size())
			throw std::runtime_error("Cannot determine executable path for engine Lua API.");
		executable.resize(length);
		return std::filesystem::path(executable).parent_path() / "LLMWiki" / "API" / "Lua";
	}

	std::string Utf8Path(const std::filesystem::path& path)
	{
		const auto utf8 = path.generic_u8string();
		return std::string(utf8.begin(), utf8.end());
	}

	// LuaJIT's default file loader uses narrow fopen on Windows. Read shared API
	// modules with a native filesystem path so Unicode installation paths work.
	int LoadEngineApiModule(lua_State* state)
	{
		const char* name = luaL_checkstring(state, 1);
		bool loadFailed = false;
		{
			const std::string moduleName(name);
			if (moduleName.empty() || moduleName.find_first_not_of(
				"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_") != std::string::npos)
			{
				lua_pushliteral(state, "\n\tNot a shared engine API module");
				return 1;
			}
			const auto path = EngineApiDirectory() / (moduleName + ".lua");
			std::ifstream file(path, std::ios::binary);
			if (!file)
			{
				lua_pushliteral(state, "\n\tNo matching shared engine API module");
				return 1;
			}
			const std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
			const std::string chunkName = "@" + Utf8Path(path);
			loadFailed = luaL_loadbuffer(state, source.data(), source.size(), chunkName.c_str()) != 0;
		}
		// Destroy C++ locals before Lua raises a load error.
		if (loadFailed) return lua_error(state);
		return 1;
	}

	void RegisterEngineApiLoader(lua_State* state)
	{
		lua_getglobal(state, "package");
		lua_getfield(state, -1, "loaders"); // LuaJIT / Lua 5.1
		const int count = static_cast<int>(lua_objlen(state, -1));
		for (int index = count; index >= 2; --index)
		{
			lua_rawgeti(state, -1, index);
			lua_rawseti(state, -2, index + 1);
		}
		lua_pushcfunction(state, LoadEngineApiModule);
		lua_rawseti(state, -2, 2); // Keep package.preload first, ahead of project modules.
		lua_pop(state, 2);
	}
	int LuaDebugLogError(lua_State* state)
	{
		lua_getglobal(state, "tostring");
		lua_pushvalue(state, 1);
		lua_call(state, 1, 1);
		const char* message = lua_tostring(state, -1);
		std::string log = message ? message : "";
		lua_pop(state, 1);

		lua_Debug caller{};
		std::string fileName = "Unknown";
		if (lua_getstack(state, 1, &caller) && lua_getinfo(state, "S", &caller) &&
			caller.source && caller.source[0] == '@')
		{
			fileName = std::filesystem::path(caller.source + 1).filename().string();
		}
		PixelLog::Error("FileName : [" + fileName + "]\nLog : " + log);
		return 0;
	}

	bool RegisterLuaDebugLogError(lua_State* state)
	{
		lua_getglobal(state, "Debug");
		if (!lua_istable(state, -1))
		{
			lua_pop(state, 1);
			return false;
		}
		lua_pushcfunction(state, LuaDebugLogError);
		lua_setfield(state, -2, "LogError");
		lua_pop(state, 1);
		return true;
	}

	int LuaEntityCreate(lua_State* state)
	{
		const char* scriptName = luaL_checkstring(state, 1);
		const unsigned int id = Entity_Create(scriptName);
		lua_pushinteger(state, static_cast<lua_Integer>(id));
		return 1;
	}

	int LuaPoolActive(lua_State* state)
	{
		const char* scriptName = luaL_checkstring(state, 1);
		const unsigned int id = Pool_Active(scriptName);
		lua_pushinteger(state, static_cast<lua_Integer>(id));
		return 1;
	}

	bool RegisterLuaEntityCreate(lua_State* state)
	{
		lua_getglobal(state, "Entity");
		if (!lua_istable(state, -1))
		{
			lua_pop(state, 1);
			return false;
		}

		lua_pushcfunction(state, LuaEntityCreate);
		lua_setfield(state, -2, "Create");
		lua_pop(state, 1);
		return true;
	}

	bool RegisterLuaPoolActive(lua_State* state)
	{
		lua_getglobal(state, "Pool");
		if (!lua_istable(state, -1))
		{
			lua_pop(state, 1);
			return false;
		}

		// 풀 확장은 Lua 객체 생성과 Spawn을 호출하므로 FFI가 아닌 Lua C 함수로 진입한다.
		lua_pushcfunction(state, LuaPoolActive);
		lua_setfield(state, -2, "Active");
		lua_pop(state, 1);
		return true;
	}
}

LuaManager::LuaManager()
{
#ifdef LUAJIT_VERSION
	std::cout << "컴파일 시점 확인: LuaJIT용으로 빌드됨!" << std::endl;
#else
	std::cout << "컴파일 시점 확인: 일반 Lua용으로 빌드됨!" << std::endl;
#endif
}

LuaManager::~LuaManager()
{
	LuaDebugger::Attach(nullptr);
}
void LuaManager::Initialize()
{
	lua = sol::state();
	LuaDebugger::Attach(lua.lua_state());
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
	const std::filesystem::path scriptRoot = EngineApiDirectory();
	sol::table package = lua["package"];
	const std::string packagePath = package["path"];
	package["path"] = Utf8Path(scriptRoot) + "/?.lua;" + packagePath;
	RegisterEngineApiLoader(lua.lua_state());

	//BindAll_GeneratedLuaModules(lua);
	// 새 프로젝트는 Engine Update로 API가 생성된 뒤 런타임을 준비한다.
	if (!std::filesystem::exists(scriptRoot / "EngineGenerate.lua") ||
		!std::filesystem::exists(scriptRoot / "Entity.lua")) return;
	ReadEngineGenerateFile();
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

void LuaManager::ImportLua(const std::string& filePath, const std::string filename, const std::string& ext)
{
	try
	{
		if (filename == "main")
		{
			sol::protected_function_result result = lua.script_file(filePath);
			if (!result.valid())
			{
				sol::error err = result;
				PixelLog::Error("Lua main file load failed: " + filePath + "\n" + err.what());
				return;
			}
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
			sol::load_result loaded = lua.load_file(filePath);
			if (!loaded.valid())
			{
				sol::error error = loaded;
				PixelLog::Error("Lua scene load failed: " + filePath + "\n" + error.what());
				return;
			}
			sol::protected_function factory = loaded;
			// Register only the definition. Scene code runs once when that scene is entered.
			luaSceneTableMap.insert({ filename, new LuaSceneInfo(factory) });
			Engine->GetFactory<SceneManager>()->CreateScene(filePath);
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
			else if (!result.valid())
			{
				sol::error err = result;
				PixelLog::Error("Lua script load failed: " + filePath + "\n" + err.what());
			}
			else
			{
				PixelLog::Error("Lua script must return a table: " + filePath);
			}
		}
		else if (ext == ".lua")
		{
			const std::string normalizedPath =
				std::filesystem::path(filePath).lexically_normal().generic_string();

			sol::protected_function_result result = lua.script_file(normalizedPath);
			if (!result.valid())
			{
				sol::error err = result;
				PixelLog::Error("Lua file load failed: " + normalizedPath + "\n" + err.what());
				return;
			}
		}
	}
	catch (const sol::error& e)
	{
		PixelLog::Error(e.what());
	}
}

void LuaManager::CreateLuaManager()
{
	HMODULE hModule = GET_CURRENT_MODULE();
	HRSRC hResInfo = FindResource(hModule, MAKEINTRESOURCE(IDR_LUA1), "LUA");
	if (!hResInfo)
	{
		return;
	}
	HGLOBAL hResData = LoadResource(hModule, hResInfo);
	void* pData = LockResource(hResData);
	DWORD dataSize = SizeofResource(hModule, hResInfo);
	std::string_view luaScript(static_cast<const char*>(pData), dataSize);
	auto result = lua.safe_script(luaScript);

	// 실패한 result의 result[0]을 먼저 읽으면 Lua 스택 내부에서 접근 위반이 날 수 있다.
	if (!result.valid())
	{
		sol::error err = result;
		PixelLog::Error("Embedded LuaManager load failed: " + std::string(err.what()));
		return;
	}

	if (result.return_count() > 0 && result[0].is<sol::table>())
	{
		luaManager = result[0];
		luaManager.set_function("CanDestroy", [](unsigned int id)
			{
				return Engine->GetFactory<SceneManager>()->GetNowScene()->CanDestroyEntity(id);
			});
		UpdateFunction = luaManager["Update"];
		AddFunction = luaManager["Add"];
	}
	else
	{
		PixelLog::Error("Embedded LuaManager did not return a table.");
	}
}

void LuaManager::ReadEngineGenerateFile()
{
	const std::filesystem::path scriptPath = EngineApiDirectory() / "EngineGenerate.lua";
	std::ifstream file(scriptPath, std::ios::binary);
	if (!file)
	{
		PixelLog::Error("Cannot read shared engine Lua API: " + Utf8Path(scriptPath));
		return;
	}
	const std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	auto loadResult = lua.load(source, "@" + Utf8Path(scriptPath));
	if (!loadResult.valid())
	{
		sol::error err = loadResult;
		Debug_LogError(err.what());
		return;
	}

	sol::protected_function script = loadResult;
	auto result = script();
	if (!result.valid())
	{
		sol::error err = result;
		Debug_LogError(err.what());
		return;
	}
	RegisterLuaDebugLogError(lua.lua_state());
	// 최초 생성 후에는 초기화 때 건너뛴 네이티브 진입점과 LuaManager도 준비한다.
	if (!UpdateFunction.valid())
	{
		if (!RegisterLuaEntityCreate(lua.lua_state()) || !RegisterLuaPoolActive(lua.lua_state()))
		{
			PixelLog::Error("Failed to register native Lua entry points after Engine Update.");
			return;
		}
		CreateLuaManager();
	}
}

void LuaManager::AddEntityID(unsigned int id, sol::table target)
{
	if (AddFunction.valid())
	{
		auto result = AddFunction(luaManager, id, target);
		if (!result.valid())
		{
			sol::error err = result;
			std::string errorMsg = err.what();
			Debug_LogError(errorMsg.c_str());
		}
	}
}

void LuaManager::RemoveEntityID(unsigned int id)
{
	// Lua에서 먼저 해제한 ID뿐 아니라 자식과 엔진 직접 삭제의 등록도 정리한다.
	if (luaManager.valid())
	{
		sol::object entitiesObject = luaManager["entities"];
		if (entitiesObject.is<sol::table>())
		{
			sol::table entities = entitiesObject.as<sol::table>();
			entities.raw_set(id, sol::nil);
		}
	}
}

void LuaManager::Update()
{
	if (UpdateFunction.valid())
	{
		if (IsPlayMode())
		{
			float DTime = GetDeltaTime();
			{
				auto result = UpdateFunction(luaManager, DTime);
				if (!result.valid())
				{
					sol::error err = result;
					std::string errorMsg = err.what();
					PixelLog::Error(errorMsg.c_str());
				}
			}
			// Lua update failed or completed: finish queued deletions after the batch.
			sol::protected_function flushPendingDestroy = luaManager["FlushPendingDestroy"];
			if (flushPendingDestroy.valid())
			{
				std::vector<unsigned int> pendingIDs;
				{
					auto flushResult = flushPendingDestroy(luaManager);
					if (!flushResult.valid())
					{
						sol::error err = flushResult;
						PixelLog::Error(err.what());
					}
					else if (flushResult.get_type() == sol::type::table)
					{
						sol::table ids = flushResult;
						for (const auto& entry : ids)
						{
							if (entry.second.is<unsigned int>()) pendingIDs.push_back(entry.second.as<unsigned int>());
						}
					}
				}
				// Native destruction can remove Lua registrations. Do it only after the
				// protected Lua call and its result have left the Lua stack.
				for (const unsigned int id : pendingIDs)
				{
					Engine->GetFactory<SceneManager>()->DestroyEntity(id);
				}
			}
		}
	}
}

void LuaManager::ResetScene()
{
	if (!luaManager.valid()) return;
	sol::protected_function reset = luaManager["ResetScene"];
	if (!reset.valid()) return;
	auto result = reset(luaManager);
	if (!result.valid())
	{
		sol::error error = result;
		PixelLog::Error("Lua scene reset failed: " + std::string(error.what()));
	}
}

void LuaManager::Release()
{
	ResetScene();

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
