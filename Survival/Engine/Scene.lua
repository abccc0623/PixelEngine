local ffi = require("ffi")
local dll = ffi.load("PixelEngine")

---@class Scene
ffi.cdef[[
	void Scene_Change(const char* sceneName);
]]

---@class Scene
Scene = Scene or {}

---@param sceneName string
function Scene.Change(sceneName)
	return dll.Scene_Change(sceneName)
end
return Scene