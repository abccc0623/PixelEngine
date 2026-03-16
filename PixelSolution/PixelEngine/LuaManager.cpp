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
#include "ObjectManager.h"
#include "SceneManager.h"
#include "SPointer.h"
#include "LuaClassInfo.h"
#include <iostream>
#include <filesystem>
#include "PixelMetaAPI.h"

#include "Module/Renderer2D.h"
#include "Module/LuaScript.h"
#include "GenerateLuaBind.h"
#define SOL_ALL_SAFETIES_ON 1 // 안전장치 활성화 (권장)

extern PixelEngine* Engine;
LuaManager::LuaManager()
{
    luaTableMap = std::unordered_map<std::string, LuaClassInfo*>();
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
        sol::lib::debug    // 디버깅 툴
    );
    bind  = Engine->GetFactory<BindManager>();
    input = Engine->GetFactory<KeyInputManager>();
    obj   = Engine->GetFactory<ObjectManager>();


    //Engine관련
   sol::table engine = lua.create_named_table("Engine");
   engine["CreateGameObject"] = sol::overload
   (
       [](std::string name)
       {
           auto k = CreateGameObject(name.c_str());
           return k;
       },
       []()
       {
           auto k = CreateGameObject();
           return k;
       }
   );
   engine["AddModule"] = [](GameObject* obj,const std::string& className) -> Renderer2D*
       {
           Module* findModule = AddModule(obj, GetClass(className));
           return reinterpret_cast<Renderer2D*>(findModule);
       };
  

   BindLua();
   BindLuaKey();
   BindLuaTime();
   BindLuaSetting();
   BindAll_GeneratedLuaModules(lua);
}


bool LuaManager::Load(const std::string& filePath)
{
    if (!std::filesystem::exists(filePath)) {
        std::cerr << "Error: Lua file not found at path: " << filePath << std::endl;
        return false;
    }
    
    //루아 파일 로드
    sol::load_result script = lua.load_file(filePath);
    if (!script.valid())
    {
        sol::error err = script;
        std::string what = err.what();
        return false;
    }
    
    //해당 스크립트 실행
    sol::protected_function_result result = script();
    if (!result.valid()) {
        sol::error err = result;
        // 실행 에러 로그: err.what()
        return false;
    }
    
    if (result.return_count() > 0 && result[0].is<sol::table>())
    {
        sol::table Proto = result[0];
        std::filesystem::path p(filePath);
        sol::table mt = lua.create_table_with(sol::meta_function::index, Proto);
    
        // 생성: 여기서 LuaClassInfo를 힙에 할당하여 관리
        LuaClassInfo* info = new LuaClassInfo(Proto, mt);
        luaTableMap.insert({ p.stem().string(), info });
    }
	return true;
}

LuaClassInfo* LuaManager::GetLua(const std::string& fileName)
{
    auto k = luaTableMap.find(fileName);
    if (k != luaTableMap.end())
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




void LuaManager::BindLua()
{
    std::string main = "";
    std::string FunctionName = "inline void BindAll_GeneratedLuaModules(sol::state& lua) \n";
    FunctionName += "{ \n";
   

    main += "#include \"sol.hpp\"\n";
    main += "#include \"Core/Module.h\"\n";
    main += "#include \"Core/GameObject.h\"\n";

    main += "#include \"Module/Transform.h\"\n";
    main += "#include \"Module/Renderer2D.h\"\n";
    main += "#include \"Module/LuaScript.h\"\n";
    main += "#include \"Module/Camera.h\"\n";
    main += "#include \"Module/DebugCamera.h\"\n";

    main += "#define SOL_ALL_SAFETIES_ON 1 \n";

    int classCount = AllClassCount();
    for (int C = 0; C < classCount; C++)
    {
        PClass* pclass = GetClassByIndex(C);
        if (pclass == nullptr) continue;

        std::string typeName = GetClassTypeName(pclass);
        int memberCount = GetClassMemberCount(pclass);
        int methodCount = GetClassMethodCount(pclass);

        FunctionName += "\tGenerate_" + typeName + "(lua); \n";
        main += "inline void Generate_"+typeName+"(sol::state& lua) \n";
        main += "{ \n";

        main += "  sol::usertype<"+typeName+"> ut = lua.new_usertype<"+typeName+">(\""+typeName+"\");\n";
        //맴버 등록
        for (int m = 0; m < memberCount; m++)
        {
            std::string memberName = GetClassMemberName(pclass, m);
            std::string memberType = GetClassMemberType(pclass, m);
            main += "  ut[\""+ memberName + "\"] = " + "&"+ typeName +"::"+memberName+";\n";
        }
        //함수 등록 
        for (int m = 0; m < methodCount; m++)
        {
            std::string methodName = GetClassMethodName(pclass, m);
           
            if (HasClassMethodFlag(pclass, m, MetaFlag::LUABIND) == false) continue;

            std::string methodReturn = GetClassMethodReturnType(pclass, m);
            int propertyCount = GetClassMethodPropertyCount(pclass, m);
            std::string propertySTR = "";
            propertySTR += "(";
            propertySTR += typeName +"& self";
            for (int p = 0; p < propertyCount; p++)
            {
               std::string propertyType = GetClassMethodGetPropertyType(pclass, m, p);
               propertySTR += ", "+propertyType + " P" + std::to_string(p);
            }
            propertySTR += (methodReturn != "void") ? ")->" + methodReturn + "\n" : ")\n";
            propertySTR += "  {\n";
            propertySTR += "  };\n";
            main += "  ut[\"" + methodName + "\"] = []" + propertySTR +"\n";
        }

        main += "} \n\n";
    }
    
    FunctionName += "} \n";
    main += FunctionName;

    std::ofstream outFile("./Asset/GenerateLuaBind.h");
    if (!outFile.is_open()) return;
    outFile << main;
    outFile.close();


    //std::string main = "";
    //int classCount = AllClassCount();
    //for (int C = 0; C < classCount; C++)
    //{
    //    PClass* pclass = GetClassByIndex(C);
    //    if (pclass == nullptr) continue;
    //
    //    //클래스 테이블 정의
    //    std::string typeName = GetClassTypeName(pclass);
    //
    //    main += "---@class " + typeName + "\n";
    //    //맴버 변수등록
    //    int memberCount = GetClassMemberCount(pclass);
    //    int methodCount = GetClassMethodCount(pclass);

        //for (int m = 0; m < memberCount; m++)
        //{
        //    std::string memberName = GetClassMemberName(pclass, m);
        //    std::string memberType = GetClassMemberType(pclass, m);
        //    main += "--- @field " + memberName + " " + ChangeLuaType(memberType) + "\n";
        //
        //
        //    mt[memberName] = sol::property(
        //        [pclass,m](void* obj, sol::this_state s) -> sol::object
        //        {
        //            void* ptr = GetClassMemberValue(pclass, m, obj);
        //            if (!ptr) return sol::make_object(s, sol::nil);
        //            std::string type = GetClassMemberType(pclass, m);
        //    
        //            if (type == "int") return sol::make_object(s, *reinterpret_cast<int*>(ptr));
        //            if (type == "float") return sol::make_object(s, *reinterpret_cast<float*>(ptr));
        //            if (type == "bool") return sol::make_object(s, *reinterpret_cast<bool*>(ptr));
        //            
        //            return sol::make_object(s, sol::nil);
        //        },
        //        [pclass, m](void* obj, sol::stack_object value)
        //        {
        //            void* ptr = GetClassMemberValue(pclass, m, obj);
        //            std::string type = GetClassMemberType(pclass, m);
        //        });
        //}
        //main += typeName + " = {} \n\n";

        //for (int m = 0; m < methodCount; m++)
        //{
        //    std::string methodReturnType = GetClassMethodReturnType(pclass, m);
        //    std::string methodName = GetClassMethodName(pclass, m);
        //
        //    if (methodName == "Create") continue;
        //    if (methodName == "Delete") continue;
        //    if (methodName == "Awake") continue;
        //    if (methodName == "Start") continue;
        //    if (methodName == "Update") continue;
        //    if (methodName == "MatrixUpdate") continue;
        //    if (methodName == "PhysicsUpdate") continue;
        //    if (methodName == "LastUpdate") continue;
        //
        //    int propertyCount = GetClassMethodPropertyCount(pclass, m);
        //    
        //    
        //    mt[methodName] = [pclass, m](void* obj, sol::variadic_args args, sol::this_state s) -> void
        //        {
        //            // 1. 루아에서 넘어온 인자(args)들을 엔진 리플렉션 시스템에 맞게 변환 (언패킹)
        //            // (주의: 엔진의 리플렉션 Invoke 함수가 void** 배열이나 특정 Argument 구조체를 받는다고 가정)
        //            std::vector<void*> invokeArgs;
        //            /*
        //            for (auto arg : args) {
        //                // 여기에 루아 타입(int, float 등)을 C++ 포인터로 변환해서 invokeArgs에 넣는 로직 필요
        //            }
        //            */
        //
        //            // 2. 엔진 리플렉션 시스템을 통해 실제 함수 호출 (Invoke)
        //            // (함수 호출 후 리턴값의 포인터를 반환받는다고 가정)
        //            //void* retPtr = InvokeClassMethod(pclass, m, obj, invokeArgs.data());
        //            //
        //            //// 3. 리턴값을 다시 루아 타입으로 변환 (패킹)
        //            //if (returnType == "void" || !retPtr) return sol::make_object(s, sol::nil);
        //            //
        //            //if (returnType == "int") return sol::make_object(s, *reinterpret_cast<int*>(retPtr));
        //            //if (returnType == "float") return sol::make_object(s, *reinterpret_cast<float*>(retPtr));
        //            //if (returnType == "bool") return sol::make_object(s, *reinterpret_cast<bool*>(retPtr));
        //            //
        //            //return sol::make_object(s, sol::nil);
        //        };
        //
        //
        //    //for (int p = 0; p < propertyCount; p++)
        //    //{
        //    //    std::string propertyType =  GetClassMethodGetPropertyType(pclass, m, p);
        //    //    main += "---@param property" + std::to_string(p) +" " + ChangeLuaType(propertyType) + "\n";
        //    //}
        //    //main += "function " + typeName + ":" + methodName +"(x, y, z)" +"end \n";
        //};
        //sol::metatable mt = lua.create_table_with();
        //sol::table clssTable = lua.create_named_table(typeName);
        //if (typeName == "Renderer2D")
        //{
        //    lua["Renderer2D"]["SetTexture"] = [](const std::string& name)
        //        {
        //            Log::Info("SetTexture: " + name);
        //        };
        //}
        //clssTable[sol::metatable_key] = mt;
    //}

    


    //std::ofstream outFile("./Asset/PixelEngineAPI.lua");
    //if (!outFile.is_open()) return;
    //outFile << main;
    //outFile.close();
}

void LuaManager::BindLuaKey()
{
    //Key관련 등록
   //sol::table input = lua->create_named_table("Input");
   //input["GetKey"] = [](int keyNumber)->bool { return GetKey(keyNumber); };
   //input["GetKeyDown"] = [](int keyNumber)->bool {return GetKeyDown(keyNumber); };
   //input["GetKeyUp"] = [](int keyNumber)->bool { return GetKeyUp(keyNumber); };
   //input["GetMousePosition_X"] = GetMousePosition_X;
   //input["GetMousePosition_Y"] = GetMousePosition_Y;
}
void LuaManager::BindLuaTime()
{
    //sol::table time = lua->create_named_table("Time");
     //time["GetDeltaTime"] = GetDeltaTime;
     //time["GetTotalTime"] = GetTotalTime;
     //time["GetFPS"] = GetFPS;
}

void LuaManager::BindLuaSetting()
{
    //
    ////Setting 관련
    //sol::table setting = lua->create_named_table("Setting");
    //setting["ChangeScene"] = [](std::string name)
    //    {
    //        auto sceneManager = Engine->GetFactory<SceneManager>();
    //        sceneManager->ChangeScene(name);
    //    };
    //setting["CreateScene"] = [](std::string luaPath)
    //    {
    //        auto sceneManager = Engine->GetFactory<SceneManager>();
    //        sceneManager->CreateScene(luaPath);
    //    };
}



sol::state* LuaManager::GetModuleCall_Lua()
{
    //if (lua != nullptr){return lua;}
    return nullptr;
}

void LuaManager::AddLuaAPI(std::string className, std::vector<std::string> functionName)
{
    apiDefinitions += "\n";
    apiDefinitions += "---@class " + className + "\n";
    apiDefinitions += className + "= {} \n";
    for (int i = 0; i < functionName.size(); i++)
    {
        apiDefinitions += functionName[i];
        apiDefinitions += " end \n";
    }
    apiDefinitions += "\n";
}



bool LuaManager::CreateLuaAPIPath(const std::string& filePath)
{
    apiExportPath = filePath;
    //GenerateSimpleStubs(lua);
    return true;
}

void LuaManager::LoadDefaultSettingFile()
{
    //만약 에셋폴더의 Setting Lua라는 파일이 없을떄 DLL에서 제공하는 파일을 사용함
    //if (!std::filesystem::exists("./Asset/Setting.lua"))
    //{
    //    std::cerr << "Start Engine DefaultSetting File" << std::endl;
    //
    //    HMODULE hModule = GetModuleHandle(L"PixelEngine.dll");
    //    HRSRC hRes = FindResource(hModule, MAKEINTRESOURCE(IDR_LUA1), L"LUA");
    //
    //    HGLOBAL hData = LoadResource(hModule, hRes);
    //    DWORD size = SizeofResource(hModule, hRes);
    //    const char* pData = (const char*)LockResource(hData);
    //
    //    if (pData && size > 0)
    //    {
    //        // pData부터 size만큼만 복사해서 안전한 C++ 문자열 생성
    //        std::string safeScript(pData, size);
    //
    //        // 에러를 상세히 보기 위해 보호된 실행(safe_script) 권장
    //        auto result = lua->safe_script(safeScript, sol::script_pass_on_error);
    //        if (!result.valid()) 
    //        {
    //            sol::error err = result;
    //            std::string what = err.what();
    //            Log::Error("루아 리소스 실행 에러: " + what);
    //        }
    //        else 
    //        {
    //            sol::table testTable = result;
    //        }
    //    }
    //}
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
    if (input->GetKeyDown(VK_OEM_3))
    {
        obj->ReloadLuaScript();
    }
}

void LuaManager::ReleaseShared()
{
    for (auto& k : luaTableMap)
    {
        delete k.second;
    }
    lua.collect_garbage();
}
