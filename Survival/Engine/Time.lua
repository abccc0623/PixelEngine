local ffi = require("ffi")
local dll = ffi.load("PixelEngine")

---@class Time
ffi.cdef[[
	float Time_GetDeltaTime();
	float Time_GetUnscaledDeltaTime();
	double Time_GetTotalTime();
	int Time_GetFPS();
	void Time_SetPaused(bool paused);
	bool Time_IsPaused();
]]

---@class Time
Time = Time or {}

---@return number
function Time.GetDeltaTime()
	return dll.Time_GetDeltaTime()
end
---@return number
function Time.GetUnscaledDeltaTime()
	return dll.Time_GetUnscaledDeltaTime()
end
---@return number
function Time.GetTotalTime()
	return dll.Time_GetTotalTime()
end
---@return number
function Time.GetFPS()
	return dll.Time_GetFPS()
end
---@param paused boolean
function Time.SetPaused(paused)
	return dll.Time_SetPaused(paused)
end
---@return boolean
function Time.IsPaused()
	return dll.Time_IsPaused()
end
return Time