local ffi = require("ffi")
local dll = ffi.load("PixelEngine")

---@class Event
ffi.cdef[[
	bool Event_Bind(unsigned int ID,const char* EventName,const char* BindFunctionName);
	bool Event_Call(const char* EventName);
]]

---@class Event
Event = Event or {}

---@param ID number
---@param EventName string
---@param BindFunctionName string
---@return boolean
function Event.Bind(ID,EventName,BindFunctionName)
	return dll.Event_Bind(ID,EventName,BindFunctionName)
end
---@param EventName string
---@return boolean
function Event.Call(EventName)
	return dll.Event_Call(EventName)
end
return Event