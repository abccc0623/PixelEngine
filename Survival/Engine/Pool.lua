local ffi = require("ffi")
local dll = ffi.load("PixelEngine")

---@class Pool
ffi.cdef[[
	unsigned int Pool_Active(const char* ScriptName);
	void Pool_Disable(const char* ScriptName,unsigned int ID);
	void Pool_Clear(const char* ScriptName);
	int Pool_GetActiveCount(const char* ScriptName);
	unsigned int Pool_GetActiveID(const char* ScriptName,int Index);
	void Pool_SetExpansionSize(const char* ScriptName,int CreateCount);
]]

---@class Pool
Pool = Pool or {}

---@param ScriptName string
---@return number
function Pool.Active(ScriptName)
	return dll.Pool_Active(ScriptName)
end
---@param ScriptName string
---@param ID number
function Pool.Disable(ScriptName,ID)
	return dll.Pool_Disable(ScriptName,ID)
end
---@param ScriptName string
function Pool.Clear(ScriptName)
	return dll.Pool_Clear(ScriptName)
end
---@param ScriptName string
---@return number
function Pool.GetActiveCount(ScriptName)
	return dll.Pool_GetActiveCount(ScriptName)
end
---@param ScriptName string
---@param Index number
---@return number
function Pool.GetActiveID(ScriptName,Index)
	return dll.Pool_GetActiveID(ScriptName,Index)
end
---@param ScriptName string
---@param CreateCount number
function Pool.SetExpansionSize(ScriptName,CreateCount)
	return dll.Pool_SetExpansionSize(ScriptName,CreateCount)
end
return Pool