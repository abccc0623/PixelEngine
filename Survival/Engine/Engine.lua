local ffi = require("ffi")
local dll = ffi.load("PixelEngine")

---@class Engine
ffi.cdef[[
	void Engine_BackgroundColor(int colorR,int colorG,int colorB);
	void Engine_SetWindowSize(int width,int height);
]]

---@class Engine
Engine = Engine or {}

---@param colorR number
---@param colorG number
---@param colorB number
function Engine.BackgroundColor(colorR,colorG,colorB)
	return dll.Engine_BackgroundColor(colorR,colorG,colorB)
end
---@param width number
---@param height number
function Engine.SetWindowSize(width,height)
	return dll.Engine_SetWindowSize(width,height)
end
return Engine