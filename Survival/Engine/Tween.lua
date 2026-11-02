local ffi = require("ffi")
local dll = ffi.load("PixelEngine")

---@class Tween
ffi.cdef[[
	bool Tween_PingPong(unsigned int ID,int Property,float MinValue,float MaxValue,float Duration,bool UseUnscaledTime,int Ease);
	bool Tween_Move(unsigned int ID,int Property,float StartValue,float EndValue,float Duration,bool UseUnscaledTime,int Ease);
	bool Tween_Pause(unsigned int ID,int Property);
	bool Tween_Resume(unsigned int ID,int Property);
	bool Tween_Kill(unsigned int ID,int Property);
]]

---@class Tween
Tween = Tween or {}

---@param ID number
---@param Property number
---@param MinValue number
---@param MaxValue number
---@param Duration number
---@param UseUnscaledTime boolean
---@param Ease number
---@return boolean
function Tween.PingPong(ID,Property,MinValue,MaxValue,Duration,UseUnscaledTime,Ease)
	Ease = Ease or 0
	return dll.Tween_PingPong(ID,Property,MinValue,MaxValue,Duration,UseUnscaledTime,Ease)
end
---@param ID number
---@param Property number
---@param StartValue number
---@param EndValue number
---@param Duration number
---@param UseUnscaledTime boolean
---@param Ease number
---@return boolean
function Tween.Move(ID,Property,StartValue,EndValue,Duration,UseUnscaledTime,Ease)
	Ease = Ease or 0
	return dll.Tween_Move(ID,Property,StartValue,EndValue,Duration,UseUnscaledTime,Ease)
end
---@param ID number
---@param Property number
---@return boolean
function Tween.Pause(ID,Property)
	return dll.Tween_Pause(ID,Property)
end
---@param ID number
---@param Property number
---@return boolean
function Tween.Resume(ID,Property)
	return dll.Tween_Resume(ID,Property)
end
---@param ID number
---@param Property number
---@return boolean
function Tween.Kill(ID,Property)
	return dll.Tween_Kill(ID,Property)
end
return Tween