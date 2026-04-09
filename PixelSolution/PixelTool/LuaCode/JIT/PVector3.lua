local ffi = require("ffi")

pcall(ffi.cdef, [[
    typedef struct { float X, Y, Z; } PVector3;
]])

local MT = {
    __index = {
        Normalize = function(self)
            local n = math.sqrt(self.X^2 + self.Y^2 + self.Z^2)
            if n > 1e-5 then 
                self.X, self.Y, self.Z = self.X/n, self.Y/n, self.Z/n 
            end
            return self
        end,
        FromUserdata = function(ud)
            return ffi.cast("PVector3*", ud)
        end
    },
    __add = function(a, b) return ffi.new("PVector3", a.X+b.X, a.Y+b.Y, a.Z+b.Z) end,
    __mul = function(a, b) 
        if type(b) == "number" then return ffi.new("PVector3", a.X*b, a.Y*b, a.Z*b) end
    end,
    __tostring = function(self)
        return string.format("Vector3(%.2f, %.2f, %.2f)", self.X, self.Y, self.Z)
    end
}
local ctor = ffi.metatype("PVector3", MT)
_G.PVector3 = ctor 

return ctor