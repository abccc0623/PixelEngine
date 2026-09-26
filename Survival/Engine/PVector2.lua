local ffi = require("ffi")
local dll = ffi.load("PixelEngine")

---@class PVector2
---@field x number
---@field y number
ffi.cdef[[
	typedef struct {
		float x;
		float y;
	} PVector2;

	PVector2 PVector2_Create(float x,float y);
]]

local PVector2_value_mt = {
	__index = {
	},
	__eq = function(a, b)
		return a.x == b.x and a.y == b.y
	end,
	__lt = function(a, b)
		return a.x < b.x and a.y < b.y
	end,
	__le = function(a, b)
		return a.x <= b.x and a.y <= b.y
	end
}

ffi.metatype("PVector2", PVector2_value_mt)

---@class PVector2
PVector2 = PVector2 or {}
setmetatable(PVector2, {
	__call = function(_,x,y)
		return dll.PVector2_Create(x,y)
	end
})

return PVector2