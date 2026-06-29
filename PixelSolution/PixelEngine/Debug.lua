local ffi = require("ffi")

ffi.cdef[[
    void Debug_Log(const char* log);
    void Debug_LogError(const char* log);
    void Debug_LogWarning(const char* log);

    void Debug_Line(float startX, float startY, float startZ, float endX, float endY, float endZ, float r, float g, float b);
    void Debug_Box2D(float x, float y, float z, float width, float height, float r, float g, float b);
    void Debug_Sphere2D(float x, float y, float z, float radius, float r, float g, float b);
]]

local dll = ffi.load("PixelEngine")

Debug = Debug or {}

---@param log string
function Debug.Log(log)
    dll.Debug_Log(tostring(log))
end

---@param log string
function Debug.LogError(log)
    dll.Debug_LogError(tostring(log))
end

---@param log string
function Debug.LogWarning(log)
    dll.Debug_LogWarning(tostring(log))
end

---@param startX number
---@param startY number
---@param startZ number
---@param endX number
---@param endY number
---@param endZ number
---@param r number
---@param g number
---@param b number
function Debug.Line(startX, startY, startZ, endX, endY, endZ, r, g, b)
    dll.Debug_Line(startX, startY, startZ, endX, endY, endZ, r, g, b)
end

---@param x number
---@param y number
---@param z number
---@param width number
---@param height number
---@param r number
---@param g number
---@param b number
function Debug.Box2D(x, y, z, width, height, r, g, b)
    dll.Debug_Box2D(x, y, z, width, height, r, g, b)
end

---@param x number
---@param y number
---@param z number
---@param radius number
---@param r number
---@param g number
---@param b number
function Debug.Sphere2D(x, y, z, radius, r, g, b)
    dll.Debug_Sphere2D(x, y, z, radius, r, g, b)
end

return Debug
