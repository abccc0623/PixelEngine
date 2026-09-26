local ffi = require("ffi")
local dll = ffi.load("PixelEngine")

ffi.cdef[[
	typedef struct {
		unsigned int thisID;
	} Animation2DData;

	Animation2DData* Animation2D_Add(unsigned int ID);
	Animation2DData* Animation2D_Get(unsigned int ID);
	bool Animation2D_Has(unsigned int ID);
	void Animation2D_Play(unsigned int ID,int AnimationIndex);
	void Animation2D_Stop(unsigned int ID);
	void Animation2D_Reset(unsigned int ID);
	int Animation2D_Create(unsigned int ID,const char* TextureName,int MaxFrameX,int MaxFrameY,float Speed);
	int Animation2D_GetFrameIndex(unsigned int ID,int AnimationIndex);
	void Animation2D_SetFrameIndex(unsigned int ID,int AnimationIndex,int FrameIndex);
	void Animation2D_SetUseUnscaledTime(unsigned int ID,bool UseUnscaledTime);
	void Animation2D_SetLoop(unsigned int ID,int AnimationIndex,bool Loop);
	void Animation2D_SetFrameCallback(unsigned int ID,int AnimationIndex,int FrameIndex,bool Repeat);
]]

---@class Animation2DData
local Animation2DData = {}

---@param AnimationIndex number
function Animation2DData:Play(AnimationIndex)
	return dll.Animation2D_Play(self.thisID,AnimationIndex)
end

function Animation2DData:Stop()
	return dll.Animation2D_Stop(self.thisID)
end

function Animation2DData:Reset()
	return dll.Animation2D_Reset(self.thisID)
end

---@param TextureName string
---@param MaxFrameX number
---@param MaxFrameY number
---@param Speed number
---@return number
function Animation2DData:Create(TextureName,MaxFrameX,MaxFrameY,Speed)
	return dll.Animation2D_Create(self.thisID,TextureName,MaxFrameX,MaxFrameY,Speed)
end

---@param AnimationIndex number
---@return number
function Animation2DData:GetFrameIndex(AnimationIndex)
	return dll.Animation2D_GetFrameIndex(self.thisID,AnimationIndex)
end

---@param AnimationIndex number
---@param FrameIndex number
function Animation2DData:SetFrameIndex(AnimationIndex,FrameIndex)
	return dll.Animation2D_SetFrameIndex(self.thisID,AnimationIndex,FrameIndex)
end

---@param UseUnscaledTime boolean
function Animation2DData:SetUseUnscaledTime(UseUnscaledTime)
	return dll.Animation2D_SetUseUnscaledTime(self.thisID,UseUnscaledTime)
end

---@param AnimationIndex number
---@param Loop boolean
function Animation2DData:SetLoop(AnimationIndex,Loop)
	return dll.Animation2D_SetLoop(self.thisID,AnimationIndex,Loop)
end

---@param AnimationIndex number
---@param FrameIndex number
---@param Repeat boolean
function Animation2DData:SetFrameCallback(AnimationIndex,FrameIndex,Repeat)
	return dll.Animation2D_SetFrameCallback(self.thisID,AnimationIndex,FrameIndex,Repeat)
end

local Animation2D_mt = {
	__index = Animation2DData
}

ffi.metatype("Animation2DData", Animation2D_mt)

Animation2D = Animation2D or {}

---@param ID number 
---@return Animation2DData 
function Animation2D.Add(ID) 
	local data = dll.Animation2D_Add(ID)
	data.thisID = ID
	return data 
end 

---@param ID number 
---@return Animation2DData 
function Animation2D.Get(ID) 
	return dll.Animation2D_Get(ID) 
end 

---@param ID number 
---@return boolean 
function Animation2D.Has(ID) 
	return dll.Animation2D_Has(ID) 
end 

return Animation2D