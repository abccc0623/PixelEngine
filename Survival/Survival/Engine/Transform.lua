local ffi = require("ffi")
local dll = ffi.load("PixelEngine")

ffi.cdef[[
	typedef struct {
		PVector3 position;
		unsigned int thisID;
		PVector3 rotation;
		unsigned int bitmask;
		PVector3 scale;
		unsigned int unused2;
	} TransformData;

	TransformData* Transform_Add(unsigned int ID);
	TransformData* Transform_Get(unsigned int ID);
	bool Transform_Has(unsigned int ID);
	void Transform_SetParent(unsigned int ID,unsigned int ParentID);
	void Transform_SetChild(unsigned int ID,unsigned int ChildID);
]]

---@class TransformData
---@field position PVector3
---@field rotation PVector3
---@field scale PVector3
local TransformData = {}

---@param ParentID number
function TransformData:SetParent(ParentID)
	return dll.Transform_SetParent(self.thisID,ParentID)
end

---@param ChildID number
function TransformData:SetChild(ChildID)
	return dll.Transform_SetChild(self.thisID,ChildID)
end

local Transform_mt = {
	__index = TransformData
}

ffi.metatype("TransformData", Transform_mt)

Transform = Transform or {}

---@param ID number 
---@return TransformData 
function Transform.Add(ID) 
	local data = dll.Transform_Add(ID)
	data.thisID = ID
	return data 
end 

---@param ID number 
---@return TransformData 
function Transform.Get(ID) 
	return dll.Transform_Get(ID) 
end 

---@param ID number 
---@return boolean 
function Transform.Has(ID) 
	return dll.Transform_Has(ID) 
end 

return Transform