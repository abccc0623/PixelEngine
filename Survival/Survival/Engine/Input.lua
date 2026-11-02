local ffi = require("ffi")
local dll = ffi.load("PixelEngine")

---@class Input
ffi.cdef[[
	bool Input_Key(Keycode keyNumber);
	bool Input_KeyDown(Keycode keyNumber);
	bool Input_KeyUp(Keycode keyNumber);
	int Input_GetMousePositionX();
	int Input_GetMousePositionY();
]]

---@class Input
Input = Input or {}

---@param keyNumber Keycode
---@return boolean
function Input.Key(keyNumber)
	return dll.Input_Key(keyNumber)
end
---@param keyNumber Keycode
---@return boolean
function Input.KeyDown(keyNumber)
	return dll.Input_KeyDown(keyNumber)
end
---@param keyNumber Keycode
---@return boolean
function Input.KeyUp(keyNumber)
	return dll.Input_KeyUp(keyNumber)
end
---@return number
function Input.GetMousePositionX()
	return dll.Input_GetMousePositionX()
end
---@return number
function Input.GetMousePositionY()
	return dll.Input_GetMousePositionY()
end
return Input