local ffi = require("ffi")
local dll = ffi.load("PixelEngine")

---@class PVector3
---@field x number
---@field y number
---@field z number
---@operator add(PVector3): PVector3
---@operator sub(PVector3): PVector3
---@operator mul(number|PVector3): PVector3
---@operator eq(PVector3): boolean
---@operator lt(PVector3): boolean
---@operator le(PVector3): boolean
---@field Normalize fun(self: PVector3): PVector3
---@field Length fun(self: PVector3): number
---@field Distance fun(self: PVector3, to: PVector3): number
---@field Dot fun(self: PVector3, rhs: PVector3): number
---@field Cross fun(self: PVector3, rhs: PVector3): PVector3
---@field Direction fun(self: PVector3, to: PVector3): PVector3
---@field Lerp fun(self: PVector3, to: PVector3, amount: number): PVector3
---@field ApproximatelyEquals fun(self: PVector3, rhs: PVector3, epsilon: number): boolean
ffi.cdef[[
	typedef struct {
		float x;
		float y;
		float z;
	} PVector3;

	PVector3 PVector3_Normalize(PVector3 normalizeVector3);
	float PVector3_Length(PVector3 vector3);
	float PVector3_Distance(PVector3 from,PVector3 to);
	float PVector3_Dot(PVector3 lhs,PVector3 rhs);
	PVector3 PVector3_Cross(PVector3 lhs,PVector3 rhs);
	PVector3 PVector3_Direction(PVector3 from,PVector3 to);
	PVector3 PVector3_Lerp(PVector3 from,PVector3 to,float amount);
	bool PVector3_ApproximatelyEquals(PVector3 lhs,PVector3 rhs,float epsilon);
	PVector3 PVector3_Create(float x,float y,float z);
]]

local PVector3_value_mt = {
	__index = {
		Normalize = function(normalizeVector3)
			return dll.PVector3_Normalize(normalizeVector3)
		end,
		Length = function(vector3)
			return dll.PVector3_Length(vector3)
		end,
		Distance = function(from,to)
			return dll.PVector3_Distance(from,to)
		end,
		Dot = function(lhs,rhs)
			return dll.PVector3_Dot(lhs,rhs)
		end,
		Cross = function(lhs,rhs)
			return dll.PVector3_Cross(lhs,rhs)
		end,
		Direction = function(from,to)
			return dll.PVector3_Direction(from,to)
		end,
		Lerp = function(from,to,amount)
			return dll.PVector3_Lerp(from,to,amount)
		end,
		ApproximatelyEquals = function(lhs,rhs,epsilon)
			return dll.PVector3_ApproximatelyEquals(lhs,rhs,epsilon)
		end,
	},
	__eq = function(a, b)
		return a.x == b.x and a.y == b.y and a.z == b.z
	end,
	__lt = function(a, b)
		return a.x < b.x and a.y < b.y and a.z < b.z
	end,
	__le = function(a, b)
		return a.x <= b.x and a.y <= b.y and a.z <= b.z
	end,
	__add = function(a, b)
		return dll.PVector3_Create(a.x + b.x, a.y + b.y, a.z + b.z)
	end,
	__sub = function(a, b)
		return dll.PVector3_Create(a.x - b.x, a.y - b.y, a.z - b.z)
	end,
	__mul = function(a, b)
		if type(a) == "number" then
			return dll.PVector3_Create(a * b.x, a * b.y, a * b.z)
		end
		if type(b) == "number" then
			return dll.PVector3_Create(a.x * b, a.y * b, a.z * b)
		end
		return dll.PVector3_Create(a.x * b.x, a.y * b.y, a.z * b.z)
	end
}

ffi.metatype("PVector3", PVector3_value_mt)

---@class PVector3
PVector3 = PVector3 or {}
setmetatable(PVector3, {
	__call = function(_,x,y,z)
		return dll.PVector3_Create(x,y,z)
	end
})

return PVector3