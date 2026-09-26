local ffi = require("ffi")
local dll = ffi.load("PixelEngine")

---@class Group
ffi.cdef[[
	unsigned int Group_CreateGroupAndEntity(const char* GroupName,const char* ScriptName);
	void Group_Set(const char* GroupName,unsigned int ID);
	void Group_Remove(const char* GroupName,unsigned int ID);
	void Group_Clear(const char* GroupName);
	int Group_Count(const char* GroupName);
	unsigned int Group_First(const char* GroupName);
	int Group_GetCount(const char* GroupName);
	unsigned int Group_GetID(const char* GroupName,int Index);
]]

---@class Group
Group = Group or {}

---@param GroupName string
---@param ScriptName string
---@return number
function Group.CreateGroupAndEntity(GroupName,ScriptName)
	return dll.Group_CreateGroupAndEntity(GroupName,ScriptName)
end
---@param GroupName string
---@param ID number
function Group.Set(GroupName,ID)
	return dll.Group_Set(GroupName,ID)
end
---@param GroupName string
---@param ID number
function Group.Remove(GroupName,ID)
	return dll.Group_Remove(GroupName,ID)
end
---@param GroupName string
function Group.Clear(GroupName)
	return dll.Group_Clear(GroupName)
end
---@param GroupName string
---@return number
function Group.Count(GroupName)
	return dll.Group_Count(GroupName)
end
---@param GroupName string
---@return number
function Group.First(GroupName)
	return dll.Group_First(GroupName)
end
---@param GroupName string
---@return number
function Group.GetCount(GroupName)
	return dll.Group_GetCount(GroupName)
end
---@param GroupName string
---@param Index number
---@return number
function Group.GetID(GroupName,Index)
	return dll.Group_GetID(GroupName,Index)
end
return Group