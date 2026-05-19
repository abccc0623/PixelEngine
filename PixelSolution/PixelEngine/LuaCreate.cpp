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
	Vector2File();
	Vector3File();
	for (auto& K : types)
	{
		switch (K.metaType)
		{
		case META_TYPE::STATIC:
			CreateComponent(K);
			break;
		}
	}
	ComponentLinkFile();
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
		int size = meta.methods.size();
		std::string function = "";
		for (int i = 0; i < size; i++)
		{
			if (meta.methods[i].name == "BindJit")
			{
				luaFile += "local ffi = require(\"ffi\")\n\n";
				PType* type = GetType(meta.thisName);
				std::vector<void*> property;
				PValue value = CallMethod(type, i, nullptr, property);
				luaFile += value.v_string;
				luaFile += "\n\n";
			}
			else if (meta.methods[i].name == "AddComponent")
			{
				std::string fun = "";
				fun += "---@param entityID number\n";
				fun += "---@return {{CLASS_NAME}}Data|nil\n";
				fun += "function {{CLASS_NAME}}.Add(entityID)\n";
				fun += "\tlocal rawPtr = {{CLASS_NAME}}.AddComponent(entityID)\n";
				fun += "\tif rawPtr == nil then return nil end\n";
				fun += "\treturn ffi.cast(\"{{CLASS_NAME}}Data*\", rawPtr)\n";
				fun += "end\n\n";
				function += ReplaceAll(fun, "CLASS_NAME", meta.thisName);
			}
			else if (meta.methods[i].name == "GetComponent")
			{
				std::string fun = "";
				fun += "---@param entityID number\n";
				fun += "---@return {{CLASS_NAME}}Data|nil\n";
				fun += "function {{CLASS_NAME}}.Get(entityID)\n";
				fun += "\tlocal rawPtr = {{CLASS_NAME}}.GetComponent(entityID)\n";
				fun += "\tif rawPtr == nil then return nil end\n";
				fun += "\treturn ffi.cast(\"{{CLASS_NAME}}Data*\", rawPtr)\n";
				fun += "end\n\n";
				function += ReplaceAll(fun, "CLASS_NAME", meta.thisName);
			}
			else if (meta.methods[i].name == "HasComponent")
			{
				std::string fun = "";
				fun += "---@param entityID number\n";
				fun += "---@return boolean\n";
				fun += "function {{CLASS_NAME}}.Has(entityID)\n";
				fun += "\treturn {{CLASS_NAME}}.HasComponent(entityID)\n";
				fun += "end\n\n";
				function += ReplaceAll(fun, "CLASS_NAME", meta.thisName);
			}
		}

		luaFile += meta.thisName + " = " + meta.thisName + " or {}\n";
		luaFile += function;
		GenerateComponentFileName += "require(\"" + meta.thisName + "\")\n";
		std::string root = GetEngineRootPath() + "\\Asset\\Engine\\" + meta.thisName + ".lua";
		std::ofstream file(root);
		file << luaFile;
		file.close();
	}

	return luaFile;
}

void LuaCreate::ComponentLinkFile()
{
	std::string root = GetEngineRootPath() + "\\Asset\\Engine\\EngineGenerate.lua";
	std::string jit = "";
	jit += GenerateComponentFileName;

	std::ofstream file(root);
	file << jit;
	file.close();
}

void LuaCreate::Vector3File()
{
	std::filesystem::path root = std::filesystem::path(GetEngineRootPath()) / "Asset" / "Engine" / "Vector3.lua";

	std::string jit = R"(
local ffi = require("ffi")

---@class Vector3
---@field x number
---@field y number
---@field z number
ffi.cdef[[
    typedef struct { float x, y, z; } Vector3;
]]

local Vector3_mt = {
    __add = function(a, b) return ffi.new("Vector3", a.x + b.x, a.y + b.y, a.z + b.z) end,
    __sub = function(a, b) return ffi.new("Vector3", a.x - b.x, a.y - b.y, a.z - b.z) end,
    __mul = function(a, b)
        if type(a) == "number" then
            return ffi.new("Vector3", a * b.x, a * b.y, a * b.z)
        elseif type(b) == "number" then
            return ffi.new("Vector3", a.x * b, a.y * b, a.z * b)
        end
    end,
	__eq = function(a, b)
        local epsilon = 0.00001
        return math.abs(a.x - b.x) < epsilon and 
               math.abs(a.y - b.y) < epsilon and 
               math.abs(a.z - b.z) < epsilon
	end,
    
    __index = {
		---@param self Vector3
        ---@return number
        Length = function(self)
            return math.sqrt(self.x * self.x + self.y * self.y + self.z * self.z)
        end,

		---@param self Vector3
        ---@return Vector3
        Normalize = function(self)
            local len = self:Length()
            if len > 0.00001 then
                return ffi.new("Vector3", self.x / len, self.y / len, self.z / len)
            end
            return ffi.new("Vector3", 0, 0, 0)
        end,

		---@param self Vector3
        ---@param other Vector3
        ---@return number
        Dot = function(self, other)
            return (self.x * other.x) + (self.y * other.y) + (self.z * other.z)
        end,

		---@param self Vector3
        ---@param other Vector3
        ---@return Vector3
        Cross = function(self, other)
            return ffi.new("Vector3",
                (self.y * other.z) - (self.z * other.y),
                (self.z * other.x) - (self.x * other.z),
                (self.x * other.y) - (self.y * other.x)
            )
        end,
		
		---@param self Vector3
        ---@return boolean
		IsZero = function(self)
            local epsilon = 0.00001
            return math.abs(self.x) < epsilon and 
                   math.abs(self.y) < epsilon and 
                   math.abs(self.z) < epsilon
		end,
    }
}

ffi.metatype("Vector3", Vector3_mt)

---@param x? number
---@param y? number
---@param z? number
---@return Vector3
Vector3 = function(x, y, z)
    return ffi.new("Vector3", x or 0, y or 0, z or 0)
end
)";

	// 파일 출력
	std::ofstream file(root);
	if (file.is_open())
	{
		file << jit;
		file.close();
		GenerateComponentFileName += "require(\"Vector3\")\n";
	}
	else
	{
		// 파일 쓰기 실패 방어 로직
		PixelLog::Error("Failed to create Vector3.lua at: " + root.string());
	}
}

void LuaCreate::Vector2File()
{
	std::filesystem::path root = std::filesystem::path(GetEngineRootPath()) / "Asset" / "Engine" / "Vector2.lua";

	std::string jit = R"(
local ffi = require("ffi")

---@class Vector2
---@field x number
---@field y number
ffi.cdef[[
    typedef struct { float x, y; } Vector2;
]]

local Vector2_mt = {
    __add = function(a, b) return ffi.new("Vector2", a.x + b.x, a.y + b.y) end,
    __sub = function(a, b) return ffi.new("Vector2", a.x - b.x, a.y - b.y) end,
    __mul = function(a, b)
        if type(a) == "number" then
            return ffi.new("Vector2", a * b.x, a * b.y)
        elseif type(b) == "number" then
            return ffi.new("Vector2", a.x * b, a.y * b)
        end
    end,
    __eq = function(a, b)
        local epsilon = 0.00001
        return math.abs(a.x - b.x) < epsilon and 
               math.abs(a.y - b.y) < epsilon
    end,
    
    __index = {
		---@param self Vector2
        ---@return number
        Length = function(self)
            return math.sqrt(self.x * self.x + self.y * self.y)
        end,

		---@param self Vector2
        ---@return Vector2
        Normalize = function(self)
            local len = self:Length()
            if len > 0.00001 then
                return ffi.new("Vector2", self.x / len, self.y / len)
            end
            return ffi.new("Vector2", 0, 0)
        end,

		---@param self Vector2
        ---@param other Vector2
        ---@return number
        Dot = function(self, other)
            return (self.x * other.x) + (self.y * other.y)
        end,
        
		---@param self Vector2
        ---@param other Vector2
        ---@return number
        Cross = function(self, other)
            return (self.x * other.y) - (self.y * other.x)
        end,

		---@param self Vector2
        ---@return boolean
        IsZero = function(self)
            local epsilon = 0.00001
            return math.abs(self.x) < epsilon and 
                   math.abs(self.y) < epsilon
        end,
    }
}

ffi.metatype("Vector2", Vector2_mt)

---@param x? number
---@param y? number
---@return Vector2
Vector2 = function(x, y)
    return ffi.new("Vector2", x or 0, y or 0)
end
)";

	// 파일 출력
	std::ofstream file(root);
	if (file.is_open())
	{
		file << jit;
		file.close();
		GenerateComponentFileName += "require(\"Vector2\")\n";
	}
	else
	{
		// 파일 쓰기 실패 방어 로직
		PixelLog::Error("Failed to create Vector2.lua at: " + root.string());
	}
}
