local ffi = require("ffi")
local dll = ffi.load("PixelEngine")

---@class Debug
ffi.cdef[[
	void Debug_Log(const char* logString);
	void Debug_LogError(const char* logString);
	void Debug_LogWarning(const char* logString);
	void Debug_Line(PVector3 startPoint,PVector3 endPoint,PVector3 color);
]]

---@class Debug
Debug = Debug or {}

---@param logString string
function Debug.Log(logString)
	return dll.Debug_Log(logString)
end
---@param logString string
function Debug.LogError(logString)
	return dll.Debug_LogError(logString)
end
---@param logString string
function Debug.LogWarning(logString)
	return dll.Debug_LogWarning(logString)
end
---@param startPoint PVector3
---@param endPoint PVector3
---@param color PVector3
function Debug.Line(startPoint,endPoint,color)
	return dll.Debug_Line(startPoint,endPoint,color)
end
return Debug