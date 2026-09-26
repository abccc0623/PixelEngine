local ffi = require("ffi")
local dll = ffi.load("PixelEngine")

ffi.cdef[[
	typedef struct {
		unsigned int thisID;
		ProjectionType Projection;
		float FovY;
		float NearZ;
		float FarZ;
		float ZoomLevel;
		float ViewportX;
		float ViewportY;
		float ViewportWidth;
		float ViewportHeight;
	} CameraData;

	CameraData* Camera_Add(unsigned int ID);
	CameraData* Camera_Get(unsigned int ID);
	bool Camera_Has(unsigned int ID);
]]

---@class CameraData
---@field Projection ProjectionType
---@field FovY number
---@field NearZ number
---@field FarZ number
---@field ZoomLevel number
---@field ViewportX number
---@field ViewportY number
---@field ViewportWidth number
---@field ViewportHeight number
local CameraData = {}

local Camera_mt = {
	__index = CameraData
}

ffi.metatype("CameraData", Camera_mt)

Camera = Camera or {}

---@param ID number 
---@return CameraData 
function Camera.Add(ID) 
	local data = dll.Camera_Add(ID)
	data.thisID = ID
	return data 
end 

---@param ID number 
---@return CameraData 
function Camera.Get(ID) 
	return dll.Camera_Get(ID) 
end 

---@param ID number 
---@return boolean 
function Camera.Has(ID) 
	return dll.Camera_Has(ID) 
end 

return Camera