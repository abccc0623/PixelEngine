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
	const std::string projectRootPath = Engine->GetEngineRootFolderPath();
	const std::u8string projectRootPathUtf8(projectRootPath.begin(), projectRootPath.end());
	const std::filesystem::path scriptRoot =
		(std::filesystem::path(projectRootPathUtf8) / "Engine").lexically_normal();

	const std::string scriptRootStr = scriptRoot.generic_string();

	std::string packagePathSetup = "package.path = package.path .. ';" + scriptRootStr + "/?.lua'";
	lua.script(packagePathSetup);

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
			else { /* 에러 처리 */ }
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
	const std::string projectRootPath = Engine->GetEngineRootFolderPath();
	const std::u8string projectRootPathUtf8(projectRootPath.begin(), projectRootPath.end());
	const std::filesystem::path scriptPath =
		(std::filesystem::path(projectRootPathUtf8) / "Engine" / "EngineGenerate.lua").lexically_normal();
	if (!std::filesystem::exists(scriptPath))
	{
		return;
	}

	const std::string scriptPathString = scriptPath.generic_string();
	auto loadResult = lua.load_file(scriptPathString);
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
		std::string test = err.what();
		Debug_LogError(err.what());
		return;
	}
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
			auto result = UpdateFunction(luaManager, DTime);
			if (!result.valid())
			{
				sol::error err = result;
				std::string errorMsg = err.what();
				PixelLog::Error(errorMsg.c_str());
			}
			// Lua update failed or completed: finish queued deletions after the batch.
			sol::protected_function flushPendingDestroy = luaManager["FlushPendingDestroy"];
			if (flushPendingDestroy.valid())
			{
				auto flushResult = flushPendingDestroy(luaManager);
				if (!flushResult.valid())
				{
					sol::error err = flushResult;
					PixelLog::Error(err.what());
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
