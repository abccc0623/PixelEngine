#include "pch.h"
#include "LuaCreate.h"
#include "PixelMetaAPI.h"
LuaCreate::LuaCreate()
{

}

LuaCreate::~LuaCreate()
{

}

void LuaCreate::Generate(const char* outPath, std::vector<PixelClassMeta>& types)
{
	for (auto& K : types)
	{
		switch (K.metaType)
		{
		case META_TYPE::STATIC:
			CreateComponent(K);
			break;
		}
	}
}

std::string GetEngineRootPath()
{
	char buffer[MAX_PATH];

	// 현재 실행 중인 exe 파일의 전체 절대 경로를 버퍼에 담습니다.
	GetModuleFileNameA(NULL, buffer, MAX_PATH);

	// 파일명(Engine.exe)을 떼어내고, 폴더 경로만 반환합니다.
	std::filesystem::path exePath(buffer);
	return exePath.parent_path().string();
}

std::string LuaCreate::CreateComponent(PixelClassMeta& meta)
{
	std::string luaFile;
	if (meta.thisName != "Engine" &&
		meta.thisName != "Scene" &&
		meta.thisName != "Asset" &&
		meta.thisName != "Input" &&
		meta.thisName != "Debug")
	{
		luaFile += "local ffi = require(\"ffi\")\n\n\n";

		luaFile += "ffi.cdef[[\n";
		if (meta.thisName == "Transform")
		{
			PType* type = GetType(meta.thisName);
			std::vector<void*> property;
			PValue value = CallMethod(type, 3, nullptr, property);
			luaFile += value.v_string;
		}
		luaFile += "]]\n\n";

		luaFile += meta.thisName + " = " + meta.thisName + " or {}\n";
		luaFile += ComponentFunctionSTR(meta.thisName);



		std::string root = GetEngineRootPath() + "\\Asset\\Engine\\" + meta.thisName + ".lua";
		std::ofstream file(root);
		file << luaFile;
		file.close();
	}

	return luaFile;
}

std::string LuaCreate::ComponentFunctionSTR(std::string name)
{
	std::string fun = "";
	fun += "function {{CLASS_NAME}}.Add(entityID)\n";
	fun += "\tlocal rawPtr = {{CLASS_NAME}}.AddComponent(entityID)\n";
	fun += "\tif rawPtr == nil then return nil end\n";
	fun += "\treturn ffi.cast(\"{{CLASS_NAME}}Data*\", rawPtr)\n";
	fun += "end\n\n";

	fun += "function {{CLASS_NAME}}.Get(entityID)\n";
	fun += "\tlocal rawPtr = {{CLASS_NAME}}.GetComponent(entityID)\n";
	fun += "\tif rawPtr == nil then return nil end\n";
	fun += "\treturn ffi.cast(\"{{CLASS_NAME}}Data*\", rawPtr)\n";
	fun += "end\n\n";

	fun += "function {{CLASS_NAME}}.Has(entityID)\n";
	fun += "\treturn {{CLASS_NAME}}.HasComponent(entityID)\n";
	fun += "end\n\n";

	return ReplaceAll(fun, "CLASS_NAME", name);
}
