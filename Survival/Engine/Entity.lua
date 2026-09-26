local ffi = require("ffi")
local dll = ffi.load("PixelEngine")

---@class Entity
ffi.cdef[[
	unsigned int Entity_Create(const char* ScriptName);
	unsigned int Entity_CreateEmpty();
	void Entity_Destroy(unsigned int ID);
	bool Entity_GetActive(unsigned int ID);
	void Entity_SetActive(unsigned int ID,bool IsActive);
]]

---@class Entity
---@field GetScript fun(ID: number): LuaInstance?
---@field SetValue fun(ID: number, variableName: string, value: any): boolean
---@field GetValue fun(ID: number, variableName: string): any
---@field CallFunction fun(ID: number, functionName: string, ...: any): boolean, any
Entity = Entity or {}

---@param ScriptName string
---@return number
function Entity.Create(ScriptName)
	return dll.Entity_Create(ScriptName)
end
---@return number
function Entity.CreateEmpty()
	return dll.Entity_CreateEmpty()
end
---@param ID number
function Entity.Destroy(ID)
	return dll.Entity_Destroy(ID)
end
---@param ID number
---@return boolean
function Entity.GetActive(ID)
	return dll.Entity_GetActive(ID)
end
---@param ID number
---@param IsActive boolean
function Entity.SetActive(ID,IsActive)
	return dll.Entity_SetActive(ID,IsActive)
end
return Entity