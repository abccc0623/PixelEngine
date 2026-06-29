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
	createFilePath = std::string(outPath);
	Vector2File();
	Vector3File();
	for (auto& K : types)
	{
		switch (K.metaType)
		{
		case META_TYPE::STATIC:
			CreateComponent(K, types);
			break;
		}
	}
	ComponentLinkFile();
}

std::string LuaCreate::CreateComponent(PixelClassMeta& meta, std::vector<PixelClassMeta>& types)
{
	std::string luaFile;
	{
		int size = meta.methods.size();
		std::string function = "";
		for (int i = 0; i < size; i++)
		{
			if (meta.methods[i].name == "BindJit")
			{
				continue;
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
			else
			{
				function += CreateMethodWrapper(meta, meta.methods[i]);
			}
		}

		luaFile += CreateCDef(meta, types);
		luaFile += meta.thisName + " = " + meta.thisName + " or {}\n";
		luaFile += function;
		GenerateComponentFileName += "require(\"" + meta.thisName + "\")\n";
		std::string root = createFilePath + "/" + meta.thisName + ".lua";
		std::ofstream file(root);
		file << luaFile;
		file.close();
	}

	return luaFile;
}

PixelClassMeta* LuaCreate::FindType(std::vector<PixelClassMeta>& types, const std::string& name)
{
	for (auto& type : types)
	{
		if (type.thisName == name)
		{
			return &type;
		}
	}
	return nullptr;
}

std::string LuaCreate::TypeChangeByCType(const std::string& type)
{
	if (type == "float") return "float";
	if (type == "double") return "double";
	if (type == "bool") return "bool";
	if (type == "int" || type == "int32_t") return "int";
	if (type == "unsigned int" || type == "uint32_t") return "uint32_t";
	if (type == "Vector2" || type == "Pixel::Vector2" || type.find("glm::vec<2") != std::string::npos) return "Vector2";
	if (type == "Vector3" || type == "Pixel::Vector3" || type.find("glm::vec<3") != std::string::npos) return "Vector3";
	if (type == "MotionType") return "MotionType";
	return "";
}

std::string LuaCreate::CreateCDef(PixelClassMeta& meta, std::vector<PixelClassMeta>& types)
{
	std::string dataName = meta.thisName + "Data";
	PixelClassMeta* dataMeta = FindType(types, dataName);
	if (dataMeta == nullptr)
	{
		return "";
	}

	if (meta.thisName == "Transform")
	{
		std::string jit = "local ffi = require(\"ffi\")\n\n";
		jit += "---@class TransformData\n";
		jit += "---@field position Vector3\n";
		jit += "---@field rotation Vector3\n";
		jit += "---@field scale Vector3\n";
		jit += "ffi.cdef[[\n";
		jit += "\ttypedef struct __attribute__((aligned(16))) \n";
		jit += "\t{ \n";
		jit += "\t\tVector3 position;\n";
		jit += "\t\tuint32_t bitmask;\n";
		jit += "\t\tVector3 rotation;\n";
		jit += "\t\tuint32_t unused1;\n";
		jit += "\t\tVector3 scale;\n";
		jit += "\t\tuint32_t unused2;\n";
		jit += "\t} TransformData;\n";
		jit += "]]\n\n";
		return jit;
	}

	std::string jit = "local ffi = require(\"ffi\")\n\n";
	jit += "---@class " + dataName + "\n";
	for (auto& member : dataMeta->members)
	{
		if (member.luaBind == false) continue;
		std::string cType = TypeChangeByCType(member.type);
		if (cType.empty()) continue;
		jit += "---@field " + member.name + " " + member.type + "\n";
	}

	jit += "ffi.cdef[[\n";
	if (meta.thisName == "Rigidbody2D")
	{
		jit += "\ttypedef enum {\n";
		jit += "\t\tStatic = 0,\n";
		jit += "\t\tKinematic = 1,\n";
		jit += "\t\tDynamic = 2,\n";
		jit += "\t} MotionType;\n\n";
	}
	jit += "\ttypedef struct \n";
	jit += "\t{ \n";

	bool hasMember = false;
	for (auto& member : dataMeta->members)
	{
		if (member.luaBind == false) continue;
		std::string cType = TypeChangeByCType(member.type);
		if (cType.empty()) continue;
		jit += "\t\t" + cType + " " + member.name + ";\n";
		hasMember = true;
	}
	if (hasMember == false)
	{
		jit += "\t\tchar __unused;\n";
	}

	jit += "\t} " + dataName + ";\n";
	jit += "]]\n\n";
	return jit;
}
std::string LuaCreate::TypeChangeByLua(const std::string& type)
{
	std::string luaType = type;
	if (luaType == "Vector2" || luaType == "Pixel::Vector2" || luaType.find("glm::vec<2") != std::string::npos)
	{
		return "Vector2";
	}
	if (luaType == "Vector3" || luaType == "Pixel::Vector3" || luaType.find("glm::vec<3") != std::string::npos)
	{
		return "Vector3";
	}

	if (luaType == "int" || luaType == "int32_t" || luaType == "float" || luaType == "double" || luaType == "char" || luaType == "unsigned int" || luaType == "uint32_t")
	{
		return "number";
	}
	if (luaType == "std::string" || luaType == "string" || luaType == "const char*")
	{
		return "string";
	}
	if (luaType == "bool")
	{
		return "boolean";
	}
	if (luaType == "void")
	{
		return "nil";
	}
	if (luaType.find("sol::function") != std::string::npos ||
		luaType.find("basic_protected_function") != std::string::npos ||
		luaType.find("protected_function") != std::string::npos)
	{
		return "function";
	}
	if (luaType.find("sol::table") != std::string::npos || luaType.find("basic_table_core") != std::string::npos)
	{
		return "table";
	}
	if (luaType.find("sol::object") != std::string::npos || luaType.find("basic_object") != std::string::npos)
	{
		return "any";
	}
	std::erase(luaType, '*');
	return luaType;
}

std::string LuaCreate::CreatePropertyList(const std::vector<std::string>& propertys)
{
	std::string content;
	for (int i = 0; i < propertys.size(); i++)
	{
		content += CreatePropertyName(propertys[i], i);
		if (i != propertys.size() - 1)
		{
			content += ", ";
		}
	}
	return content;
}


std::string LuaCreate::CreatePropertyName(const std::string& type, int index)
{
	std::string luaType = TypeChangeByLua(type);
	std::erase(luaType, '*');
	std::erase(luaType, '&');
	std::erase(luaType, ' ');
	std::erase(luaType, ':');
	if (luaType == "Vector2") luaType = "vector2";
	else if (luaType == "Vector3") luaType = "vector3";
	if (luaType.empty() || luaType == "nil")
	{
		luaType = "value";
	}
	return luaType + std::to_string(index);
}

std::string LuaCreate::CreateMethodWrapper(PixelClassMeta& meta, PixelMethodMeta& method)
{
	std::string fun;
	std::string args = CreatePropertyList(method.propertys);
	std::string nativeName = "__" + meta.thisName + "_" + method.name;

	fun += "local " + nativeName + " = " + meta.thisName + "." + method.name + "\n";
	for (int i = 0; i < method.propertys.size(); i++)
	{
		fun += "---@param " + CreatePropertyName(method.propertys[i], i) + " " + TypeChangeByLua(method.propertys[i]) + "\n";
	}
	if (method.returnType != "void")
	{
		fun += "---@return " + TypeChangeByLua(method.returnType) + "\n";
	}
	fun += "function " + meta.thisName + "." + method.name + "(" + args + ")\n";
	if (method.returnType == "void")
	{
		fun += "\t" + nativeName + "(" + args + ")\n";
	}
	else
	{
		fun += "\treturn " + nativeName + "(" + args + ")\n";
	}
	fun += "end\n\n";
	return fun;
}
void LuaCreate::ComponentLinkFile()
{
	std::string root = createFilePath + "/EngineGenerate.lua";
	std::string jit = "";
	jit += GenerateComponentFileName;

	std::ofstream file(root);
	file << jit;
	file.close();
}

void LuaCreate::Vector3File()
{
	std::filesystem::path root = createFilePath + "/Vector3.lua";

	std::string jit = R"(
local ffi = require("ffi")

---@class Vector3
---@field x number
---@field y number
---@field z number
---@field Length fun(self: Vector3): number
---@field Normalize fun(self: Vector3): Vector3
---@field Dot fun(self: Vector3, other: Vector3): number
---@field Cross fun(self: Vector3, other: Vector3): Vector3
---@field Direction fun(self: Vector3, target: Vector3): Vector3
---@field Distance fun(self: Vector3, target: Vector3): number
---@field IsZero fun(self: Vector3): boolean
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
        ---@param target Vector3
        ---@return Vector3
        Direction = function(self, target)
            return ffi.new("Vector3", target.x - self.x, target.y - self.y, target.z - self.z):Normalize()
        end,

		---@param self Vector3
        ---@param target Vector3
        ---@return number
        Distance = function(self, target)
            local dx = target.x - self.x
            local dy = target.y - self.y
            local dz = target.z - self.z
            return math.sqrt(dx * dx + dy * dy + dz * dz)
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

---@type fun(x?: number, y?: number, z?: number): Vector3
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
	std::filesystem::path root = createFilePath + "/Vector2.lua";

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
