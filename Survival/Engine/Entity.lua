local ffi = require("ffi")
local dll = ffi.load("PixelEngine")
require("UIImage")

---@class Entity
ffi.cdef[[
	unsigned int Entity_Create(const char* ScriptName);
	unsigned int Entity_CreateEmpty();
	unsigned int Entity_CreateCamera();
	UIImageData* Entity_Create2DImage(const char* textureName);
	void Entity_Destroy(unsigned int ID);
	bool Entity_GetActive(unsigned int ID);
	void Entity_SetActive(unsigned int ID,bool IsActive);
]]

---@class Entity
---@field GetScript fun(ID: number): LuaInstance?
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
---Transform과 Camera를 추가한 객체를 생성하고 ID를 반환한다.
---@return number
function Entity.CreateCamera()
	return dll.Entity_CreateCamera()
end
---Transform과 UIImage를 추가하고 텍스처를 지정한 뒤 UIImageData를 반환한다.
---@param textureName string
---@return UIImageData
function Entity.Create2DImage(textureName)
	return dll.Entity_Create2DImage(textureName)
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
